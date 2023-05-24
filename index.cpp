#include "index.hpp"
#include <iostream>

/**
 * Processes every page in xml and populates indexer data structures
*/
int Index::process_xml() {
    xml_document doc;

    if (!doc.load_file(xml_filepath)) {
        return -1; // failure
    }

    xpath_node_set nodes = doc.select_nodes("/xml/page");; // FIX THIS

    for (xpath_node node: nodes) {
        all_pages.push_back(node.node());
    }

    batch_pages();
    batch_relevance();
    batch_weights();
    calculate_page_ranks();

    return 0; // success!
}

/**
 * Partitions all pages into batches for multi-threaded computation of text processing
*/
void Index::batch_pages() {
    int batch_size = all_pages.size() / 10;
    thread doc_threads[10];

    for (int i = 0; i < 10; i++) {
        doc_threads[i] = thread(&Index::process_pages, this, i, batch_size);
    }

    for (int i = 0; i < 10; i++) {
        doc_threads[i].join();
    }
}

/**
 * Processes the text for one batch of pages
 * @param batch_num: n-th batch
 * @param batch_size: size of batch
*/
void Index::process_pages(int batch_num, int batch_size) {
    int start_index = batch_num * batch_size;

    // if on last batch, need to overcompensate since batches are too small
    if (batch_num + 1 == 10) {
        batch_size += all_pages.size() % 10;
    } 

    for (int i = start_index; i < start_index + batch_size; i++) {
        xml_node page = all_pages[i];
        int id = stoi(trim(page.child("id").text().get()));
        string title = lower(trim(page.child("title").text().get()));
        string text  = lower(trim(page.child("text").text().get()));
        int d_id = id % 10; // lock d_id-th mutex!
        titles_to_ids[d_id][title] = id;
        doc_mutexes[d_id].lock();
        titles_to_processed_text[d_id][title] = process_text(title, text, d_id);
        doc_mutexes[d_id].unlock();
    }
}

/**
 * Processes the text for a single document
 * @param title: title of doc to process
 * @param text: text of doc to process
 * @param d_id: mutex number to lock
 * @return processed text as dict of words -> counts
*/
unordered_map<string, int> Index::process_text(string title, string text, int d_id) {
    vector<string> all_tokens = processor.tokenize(title);
    vector<string> text_tokens = processor.tokenize(text);
    all_tokens.insert(all_tokens.end(), text_tokens.begin(), text_tokens.end()); // combine to get all tokens!
    unordered_map<string, int> processed_text;
    int max_count = 0;

    while (all_tokens.size() > 0) {
        string token = all_tokens.back();
        all_tokens.pop_back();

        if (processor.is_link(token)) {
            vector<string> new_tokens = extract_tokens_from_link(token.substr(2, token.size() - 4), title, d_id); 
            all_tokens.insert(all_tokens.end(), new_tokens.begin(), new_tokens.end()); // add tokens to be processed
        }
        else if (!processor.is_stop_word(token)) {
            string stemmed_word = processor.stem_word(token);
            
            // this should only happen once for a given word in this doc!
            if (processed_text.count(stemmed_word) == 0) {
                int w_id = (abs(stemmed_word.front() - 97) % 26) / 2;
                word_mutexes[w_id].lock();
                words_to_doc_counts[w_id][stemmed_word] += 1;
                word_mutexes[w_id].unlock();
            }

            processed_text[stemmed_word] += 1;
            max_count = max(max_count, processed_text[stemmed_word]);
        }
    }

    titles_to_max_counts[d_id][title] = max_count;

    return processed_text;
}

/**
 * Produces vector of tokens from a given link and updates graph of page linking relationships
 * @param link: the link to be tokenized
 * @param title: the title of the document containing the link
 * @param d_id: mutex number to lock
 * @return vector of tokens produced from the link
 * (need to be careful about this method for find)
*/
vector<string> Index::extract_tokens_from_link(string link, string title, int d_id) {
    if (link.find('|') != string::npos) {
        string left = split_string(link, '|')[0]; // links to this title, non-tokenized
        vector<string> right = processor.tokenize(split_string(link, '|')[1]); // only want text right of the "|" as tokens
        page_weights[d_id][title][left] = 1; // placeholder

        return right;
    }
    else if (link.find("Category:") != string::npos) {
        vector<string> tokens = processor.tokenize(link.substr(link.find("Category:") + 9));
        tokens.push_back("category");
        page_weights[d_id][title][link] = 1; // placeholder

        return tokens;
    }
    else {
        page_weights[d_id][title][link] = 1; // placeholder

        return processor.tokenize(link);
    }
}

/**
 * Partitions all words into batches for multi-threaded computation
*/
void Index::batch_relevance() {
    double n = calculate_n();
    thread word_threads[26];

    for (int i = 0; i < 26; i++) {
        word_threads[i] = thread(&Index::calculate_relevance, this, i, n); // i is w_id!
    }

    for (int i = 0; i < 26; i++) {
        word_threads[i].join();
    }
}

/**
 * Calculates the relevance between all terms in the corpus in a given bucket and all documents
 * @param w_id: mutex id for this current bucket of words
 * @param n: number of total documents in the corpus
*/
void Index::calculate_relevance(int w_id, double n) {
    for (const auto& x: words_to_doc_counts[w_id]) {
        string word = x.first;
        double idf = log(n / words_to_doc_counts[w_id][word]);

        // for each doc id bucket
        for (int d_id = 0; d_id < 10; d_id++) {
            for (const auto& y: titles_to_ids[d_id]) {
                string title = y.first;

                if (titles_to_processed_text[d_id][title].count(word) > 0) {
                    double tf = (double) titles_to_processed_text[d_id][title][word] / titles_to_max_counts[d_id][title];
                    term_relevances[w_id][word][title] = tf * idf;
                }
            }
        }
    }
}


/**
 * Partitions all pages into batches for multi-threaded computation of weights
*/
void Index::batch_weights() {
    double epsilon = 0.15;
    double n = calculate_n();
    thread doc_threads[10];

    for (int i = 0; i < 10; i++) {
        doc_threads[i] = thread(&Index::calculate_weights, this, i, n, epsilon); // i is d_id!
    }

    for (int i = 0; i < 10; i++) {
        doc_threads[i].join();
    }
}

/**
 * Calculates and populates page_weights
 * @param d_id: mutex number to lock
 * @param n: number of total documents in the corpus
 * @param epsilon: hyperparameter for weight calculations
*/
void Index::calculate_weights(int d_id, double n, double epsilon) {
    for (const auto& x: page_weights[d_id]) {
        string start_title = x.first;
        int nk = calculate_nk(start_title, d_id);
        bool links_to_nothing = false;

        if (nk == 0) {
            links_to_nothing = true;
            nk = n - 1;
        }

        // for each doc bucket (nested)
        for (int j = 0; j < 10; j++) {
            for (const auto& y: titles_to_ids[j]) {
                string end_title = y.first;
                
                if (start_title != end_title && (links_to_nothing || page_weights[d_id][start_title][end_title] != 0)) {
                    page_weights[d_id][start_title][end_title] = (epsilon / n) + ((1 - epsilon) / nk);
                }
                else {
                    page_weights[d_id][start_title][end_title] = epsilon / n;
                }
            }
        }
    }
}

/**
 * Calculates the page ranks for all documents
*/
void Index::calculate_page_ranks() {
    double n = calculate_n();
    double delta = 0.001;
    unordered_map<string, double> prev;
    unordered_map<string, double> curr;

    // for each doc bucket!
    for (int d_id = 0; d_id < 10; d_id++) {
        for (const auto& x: titles_to_ids[d_id]) {
            string title = x.first;
            prev[title] = 0;
            curr[title] = 1 / n;
        }
    }
    
    while (euclidean_distance(prev, curr) > delta) {
        prev = curr; // creates a copy!

        // for each doc bucket!
        for (int d_id = 0; d_id < 10; d_id++) {
            for (const auto& x: titles_to_ids[d_id]) {
                string end_title = x.first;
                double total = 0;

                // for each doc bucket (nested)
                for (int j = 0; j < 10; j++) {
                    for (const auto& y: titles_to_ids[j]) {
                        string start_title = y.first;
                        total += page_weights[j][start_title][end_title] * prev[start_title];
                    }
                }

                curr[end_title] = total;
            }
        }
    }

    page_ranks = curr; // we're done! create another copy
}

/**
 * Calculates n, or the number of documents in the corpus
 * @return n
*/
int Index::calculate_n() {
    int n = 0;

    // for each doc bucket
    for (int i = 0; i < 10; i++) {
        n += titles_to_ids[i].size();
    }

    return n;
}

/**
 * Calculates nk for a given page, where nk the number of (unique) pages that k links to
 * @param start_title: title of document to calculate for
 * @param d_id: mutex number to lock
 * @returns value of nk for the given document
*/
int Index::calculate_nk(string start_title, int d_id) {
    int nk = 0;

    for (const auto& x: page_weights[d_id][start_title]) {
        string end_title = x.first;
        int j = find_d_id(end_title); // inner d_id!
        
        if (start_title != end_title && j != -1 && titles_to_ids[j].count(end_title) > 0) {
            nk += 1;
        }
    }

    return nk;
}

/**
 * Calculates the Euclidean distance between two vectors
 * @param v1: first vector
 * @param v2: second vector
 * @return euclidean distance between v1 and v2
*/
double Index::euclidean_distance(unordered_map<string, double> v1, unordered_map<string, double> v2) { 
    double total = 0;
    assert(v1.size() == v2.size());

    for (const auto& x: v1) {
        string title = x.first;
        total += pow(v2[title] - v1[title], 2);
    }

    return sqrt(total);
}

/**
 * Finds the mutex id corresponding to a given title
 * @param title: title to find id of
 * @return id of the mutex for this title, or -1 if not found
*/
int Index::find_d_id(string title) {
    for (int d_id = 0; d_id < 10; d_id++) {
        if (titles_to_ids[d_id].count(title) > 0) {
            return d_id;
        }
    }

    return -1; // not found!
}