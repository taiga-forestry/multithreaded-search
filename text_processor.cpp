#include "text_processor.hpp"

/**
 * Constructor for Processor
*/
Processor::Processor() {
    STOP_WORDS = fill_stopwords();
    regex_str = "\\[\\[[^\\[]+?\\]\\]|[a-zA-Z0-9]+'[a-zA-Z0-9]+|[a-zA-Z0-9]+";
}

/**
 * Fills STOP_WORDS set of Processor from stopwords.txt
 * @return unordered set of stopwords
*/
unordered_set<string> Processor::fill_stopwords() {
    ifstream file("nltk/stopwords.txt");
    unordered_set<string> ret;
    
    if (file.is_open()) {
        string line;

        while (getline(file, line)) {
            ret.insert(line.c_str());
        }

        file.close();
    }

    return ret;
}

/**
 * Stems english word using porter stemming algorithm
 * @param word: word to stem
 * @return stemmed word
*/
string Processor::stem_word(string& word) {
    Porter2Stemmer::stem(word);

    return string(word);
}

/**
 * Produces all tokens for a given text
 * @param text: string to tokenize
 * @return a vector of strings (all tokens)
*/
vector<string> Processor::tokenize(const string& text) {
    vector<string> tokens;
    smatch match;
    regex pattern(regex_str);
    string::const_iterator search_start(text.cbegin());

    while (regex_search(search_start, text.cend(), match, pattern)) {
        tokens.push_back(lower(match.str()));
        search_start = match.suffix().first;
    }

    return tokens;
}

/**
 * Determines if a given token is a link
 * @param token: token to check 
 * @return true if token is link, false otherwise
*/
bool Processor::is_link(const string& token) {
    return token.size() >= 4 && token.substr(0, 2) == "[[" && token.substr(token.size() - 2, 2) == "]]";
}

/**
 * Determines whether a given token is a stop word
 * @param token: token to check 
 * @return true if token is a stop word, false otherwise
*/
bool Processor::is_stop_word(const string& token) {
    return STOP_WORDS.count(token) > 0;
}