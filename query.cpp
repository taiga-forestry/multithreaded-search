#include "query.hpp"

/**
 * Constructor for Query
*/
Query::Query() {
    index.process_xml();
}

/**
 * Tokenizes input query and produces tokens for scoring
 * @param input: string to process
 * @return vector of tokens
*/
vector<string> Query::tokenize_input(string input) {
    vector<string> tokens;

    for (string token: index.processor.tokenize(input)) {
        if (!index.processor.is_stop_word(token)) {
            tokens.push_back(index.processor.stem_word(token));
        }
    }

    return tokens;
}

/**
 * Calculates scores by summing the term-document scores for all terms in the query
 * @param processed_tokens: all terms in the query
 * @param use_page_rank: whether to include pagerank or not in scoring
*/
void Query::calculate_scores(vector<string> processed_tokens, bool use_page_rank) {
    for (const auto& x: index.page_ranks) {
        string title = x.first;
        double score = 0;

        for (string& word: processed_tokens) {
            int w_id = ((word.front() - 97) % 26) / 2;
            score += index.term_relevances[w_id][word][title];
        }

        document_scores[title] = score;

        if (use_page_rank) {
            document_scores[title] *= index.page_ranks[title];
        }
    }
}

/**
 * Prints the 10 highest-scored documents matching with the query
*/
void Query::rank_documents() {
    for (int i = 0; i < 10; i++) {
        double max_score = 0;
        string max_title = "";

        for (const auto& x: document_scores) {
            string title = x.first;

            if (document_scores[title] > max_score) {
                max_score = document_scores[title];
                max_title = title;
            }
        }

        if (max_title == "") {
            if (i == 0) {
                cout << "NO SEARCH RESULTS MATCHED YOUR QUERY. TRY AGAIN. \n";
            }

            break;
        }
        else {
            cout << i + 1 << ": " << max_title << '\n';
            document_scores.erase(max_title);
        }
    }
}