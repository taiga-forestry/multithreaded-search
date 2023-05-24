#include <unordered_map>
#include <string>
#include <iostream>
#include <mutex>
#include <assert.h>
#include <cmath>
#include "processor/text_processor.hpp"
#include "index.hpp"
using std::unordered_map;
using std::string;
using std::cout;
using std::log;
using std::min;
using pugi::xml_document;
using pugi::xml_node;

// locking scheme: by first letter of title
class Query {
    private:
        Index index; // Indexer object
        unordered_map<string, double> document_scores; // titles -> document scores

    public:
        Query();
        vector<string> tokenize_input(string input);
        void calculate_scores(vector<string> processed_tokens, bool use_page_rank);
        void rank_documents();
};