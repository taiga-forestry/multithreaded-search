#include "query.hpp"
using std::getline;
using std::cin;
using std::cout;

int main() {
    Query query;
    string input;

    while (true) {
        cout << "search> ";
        getline(cin, input);

        if (input == ":quit") {
            break;
        }

        vector<string> tokens = query.tokenize_input(input);
        query.calculate_scores(tokens, true); // always pagerank!
        query.rank_documents();
    }
}