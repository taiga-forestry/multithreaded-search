#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <unordered_set>
#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include "stemmer/porter2_stemmer.hpp"
#include "util/util.hpp"
using std::unordered_set;
using std::string;
using std::ifstream;
using std::vector;
using std::regex;
using std::tolower;
using std::smatch;

class Processor {
    private:
        unordered_set<string> STOP_WORDS;
        string regex_str;

    public:
        Processor();
        unordered_set<string> fill_stopwords();
        string stem_word(string& word);
        vector<string> tokenize(const string& text);
        bool is_link(const string& token);
        bool is_stop_word(const string& token);
};

#endif // PROCESSOR_H