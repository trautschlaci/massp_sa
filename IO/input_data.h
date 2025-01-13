#ifndef INPUT_DATA_H
#define INPUT_DATA_H

#include <vector>
#include <tuple>
#include <string>

using namespace std;


class InputData {
    static string search_file(fstream& data_file, const string& search_string);
    static tuple<int, int> read_employee_line(const string& line);
    void initialize_cover_requirements();
    void add_cover_info(const string& line);
public:
    const string& file_name;

    explicit InputData(const string& file_name);

    int num_periods;
    int num_tasks;
    int num_emps;

    int horizon_end;

    vector<tuple<int, int>> employee_workloads;
    vector<vector<tuple<int, int>>> cover_requirements;
};


#endif //INPUT_DATA_H
