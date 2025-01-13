#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <numeric>
#include <ctime>
#include <sstream>
#include <windows.h>
#include <direct.h>
#include <unordered_map>

using namespace std;


inline vector<int> create_range(int start, int end) {
    vector<int> range(end - start + 1);
    iota(range.begin(), range.end(), start);
    return range;
}

inline string create_output_directory_name()
{
    time_t now = time(nullptr);
    tm* currentDateTime = localtime(&now);
    stringstream ss;
    ss << "IO/output/" << currentDateTime->tm_year + 1900 << "_"
       << currentDateTime->tm_mon + 1 << "_"
       << currentDateTime->tm_mday << " "
       << currentDateTime->tm_hour << "_"
       << currentDateTime->tm_min << "_"
       << currentDateTime->tm_sec;
    string output_dir = ss.str();
    return output_dir;
}

inline void create_output_directory(const string& dir_name) {
    _mkdir(dir_name.c_str());
}


inline vector<string> get_problem_files(const string& path) {
    vector<string> problems;
    string searchPath = path + "\\*";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "Error opening directory: " << path << endl;
        return problems;
    }
    do {
        string fileName = findFileData.cFileName;
        if (fileName != "." && fileName != "..") {
            problems.push_back(fileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
    return problems;
}


inline unordered_map<string, string> parse_args(int argc, char* argv[]) {
    unordered_map<string, string> args;

    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            string key = argv[i];
            if (key.rfind("--", 0) == 0) {
                args[key.substr(2)] = argv[i + 1];
            } else {
                cerr << "Invalid argument: " << key << endl;
                exit(1);
            }
        } else {
            cerr << "Missing value for argument: " << argv[i] << endl;
            exit(1);
        }
    }

    return args;
}


template <typename T> T convert(const string& value) {
    istringstream iss(value);
    T result;
    iss >> result;
    return result;
}


#endif //UTILS_H
