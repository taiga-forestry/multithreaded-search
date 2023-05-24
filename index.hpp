#include <unordered_map>
#include <string>
#include <array>
#include <thread>
#include <vector>
#include <shared_mutex>
#include <assert.h>
#include <cmath>
#include "pugixml/pugixml.hpp"
#include "processor/text_processor.hpp"
using std::unordered_map;
using std::array;
using std::shared_mutex;
using std::thread;
using std::string;
using std::log;
using std::stoi;
using std::max;
using std::abs;
using std::tuple;
using pugi::xml_document;
using pugi::xml_node;
using pugi::xpath_node;
using pugi::xpath_node_set;

// forward declaration to avoid recursive dependencies
class Query;

class Index {
    private:
        friend class Query; // Query class can access Index fields
        const char* xml_filepath = "xml/MedWiki.xml"; // sys.argv[1]
        Processor processor; // text processor object
        vector<xml_node> all_pages; // vector of id, title, text of pages!

        array<shared_mutex, 10> doc_mutexes; // 10 mutexes, for last digit of doc id
        array<shared_mutex, 26> word_mutexes; // 26 mutexes, for ((ch - 97) % 26), where ch is first letter of word

        array<unordered_map<string, int>, 10> titles_to_ids; // THREAD-SAFE | titles -> ids
        array<unordered_map<string, int>, 10> titles_to_max_counts; // THREAD-SAFE | titles -> max num of occurences of any word
        unordered_map<string, double> page_ranks; // titles -> page ranks

        array<unordered_map<string, int>, 26> words_to_doc_counts; // THREAD-SAFE | words -> num of docs w/ this word
        array<unordered_map<string, unordered_map<string, double>>, 26> term_relevances; // THREAD-SAFE | words -> titles -> relevances

        array<unordered_map<string, unordered_map<string, double>>, 10> page_weights; // THREAD-SAFE | start titles -> end titles -> weights
        array<unordered_map<string, unordered_map<string, int>>, 10> titles_to_processed_text; // THREAD-SAFE | titles -> words -> counts 

    public:
        int process_xml();
        int find_d_id(string title);
        int calculate_n();
        int calculate_nk(string start_title, int d_id);

        void batch_pages();
        void process_pages(int start_index, int batch_size);
        vector<string> extract_tokens_from_link(string link, string title, int d_id);
        unordered_map<string, int> process_text(string title, string text, int d_id);

        void batch_relevance();
        void calculate_relevance(int d_id, double n);

        void batch_weights();
        void calculate_weights(int d_id, double n, double epsilon);
        void calculate_page_ranks();
        double euclidean_distance(unordered_map<string, double> v1, unordered_map<string, double> v2);
};
