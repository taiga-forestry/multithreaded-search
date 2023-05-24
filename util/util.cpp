#include "util.hpp"

auto start = std::chrono::system_clock::now(); // start time
auto end = std::chrono::system_clock::now(); // end time

/**
 * Splits string on given delimiter
 * @param str: string to split
 * @param delimiter: delimiter to split string with
 * @return vector of strings split on delimiter
*/
vector<string> split_string(string str, char delimiter) {   
    vector<string> ans;
    string curr_word = "";

    for (auto letter: str) {
        if (letter == delimiter) {
            ans.push_back(curr_word);
            curr_word = "";
        }
        else {
            curr_word += letter;
        }
    }

    ans.push_back(curr_word);

    return ans;
}

/**
 * Converts string to lowercase
 * @param str: string to convert
 * @return string as lowercase
*/
string lower(const string& str) {
    string result = str;

    for (char& c : result) {
        c = tolower(c);
    }

    return result;
}

/**
 * Strips leading and trailing whitespace for strings
 * @param str: string to strip
 * @return string with appropriate whitespace removed
*/
string trim(const string& str) {
    const string whitespace = " \t\n";
    const auto str_start = str.find_first_not_of(whitespace);

    if (str_start == string::npos) {
        return ""; // no content
    }

    const auto str_end = str.find_last_not_of(whitespace);

    return str.substr(str_start, str_end - str_start + 1);
}

/**
 * Start the timer
*/
void start_timer() {
    start = std::chrono::system_clock::now();
}

/**
 * End the timer and display elapsed time
*/
void stop_timer() {
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
}