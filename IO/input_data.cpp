#include "input_data.h"
#include <fstream>
#include <string>
#include <sstream>
#include "problem_constants.h"

using namespace std;


InputData::InputData(const string& file_name) : file_name(file_name) {
    fstream data_file;
    data_file.open(file_name, ios::in);
    if (data_file.is_open()){

        string line = search_file(data_file, "SECTION_HORIZON");
        num_periods = stoi(line);

        horizon_end = cut_off_time + num_time_slots_per_day * num_periods;

        line = search_file(data_file, "SECTION_TASKS");
        num_tasks = stoi(line);

        line = search_file(data_file, "SECTION_STAFF");
        employee_workloads.push_back(read_employee_line(line));
        while (getline(data_file, line)) {
            if (line.length() <= 1)
                break;

            employee_workloads.push_back(read_employee_line(line));
        }
        num_emps = static_cast<int>(employee_workloads.size());

        initialize_cover_requirements();

        line = search_file(data_file, "SECTION_COVER");
        add_cover_info(line);
        while (getline(data_file, line)) {
            if (line.length() <= 1)
                break;

            add_cover_info(line);
        }

        data_file.close();
    }
}

string InputData::search_file(fstream& data_file, const string& search_string) {
    string line;
    while (getline(data_file, line)) {
        if (line.find(search_string) != string::npos) {
            break;
        }
    }
    while (getline(data_file, line)) {
        if (line.find('#') == string::npos) {
            break;
        }
    }
    return line;
}

tuple<int, int> InputData::read_employee_line(const string& line) {
    auto split_line = line.substr(line.find(',') + 1);
    auto split = split_line.find(',');
    return {stoi(split_line.substr(0, split)) / 15,  stoi(split_line.substr(split + 1)) / 15};
}

void InputData::initialize_cover_requirements() {
    cover_requirements.reserve((num_periods + 1) * 24 * 4);
    for(int d = 0; d < num_periods + 1; ++d) {
        for(int s = 0; s < 24 * 4; ++s) {
            vector<tuple<int, int>> slot_vector;
            slot_vector.reserve(num_tasks);
            for(int t = 0; t < num_tasks; ++t) {
                slot_vector.emplace_back(0, 0);
            }
            cover_requirements.emplace_back(slot_vector);
        }
    }
}

void InputData::add_cover_info(const string& line) {
    stringstream ss(line);
    string str;

    getline(ss, str, ',');
    int day = stoi(str) - 1;
    getline(ss, str, ',');
    auto start = str.substr(0, str.find('-'));
    getline(ss, str, ',');
    int task = stoi(str) - 1;
    getline(ss, str, ',');
    int min_cover = stoi(str);
    getline(ss, str, ',');
    int max_cover = stoi(str);

    auto char_pos = start.find(':');
    int slot_hour = stoi(start.substr(0, char_pos));
    int slot_minute = stoi(start.substr(char_pos + 1)) / 15;
    int slot_time = (slot_hour * 4 + slot_minute) + day * 4 * 24;

    cover_requirements[slot_time][task] = {min_cover, max_cover};
}

