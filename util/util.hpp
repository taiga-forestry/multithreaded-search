#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iostream>
#include <algorithm>
using std::cout;
using std::vector;
using std::string;
using std::tolower;
using std::time_t;
using std::localtime;
using std::tm;
using std::ctime;

vector<string> split_string(string str, char delimiter);
string lower(const string& str);
string trim(const string& str);
void start_timer();
void stop_timer();

#endif // UTIL_H