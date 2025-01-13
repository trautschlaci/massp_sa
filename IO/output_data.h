#ifndef OUTPUT_DATA_H
#define OUTPUT_DATA_H

#include "input_data.h"
#include "pugixml.hpp"

using namespace std;


class ScheduledActivity
{
public:
    int task = 0;
    int length = 0;
};



class ScheduledShift
{
public:
    int activity_count = 0;
    int start = 0;

    vector<ScheduledActivity> activities;
};



class OutputData {
    int num_periods;
    int num_emps;

    static pugi::xml_node create_solution_tag(pugi::xml_document& doc, const string& problem_definition_dir);
    void create_employee_element(pugi::xml_node& solution_data, int emp_id) const;
    static void create_task_element(pugi::xml_node& shift_element, int task, int task_length);
public:
    long long int cost = -1;

    vector<vector<ScheduledShift>> shifts;


    explicit OutputData(const InputData& inputData, int max_activity_per_shift) : num_periods(inputData.num_periods), num_emps(inputData.num_emps)
    {
        shifts.resize(inputData.num_periods+1);
        for (int d = 0; d < inputData.num_periods+1; ++d)
        {
            shifts[d].resize(inputData.num_emps);
            for (int e = 0; e < inputData.num_emps; ++e)
            {
                auto& shift = shifts[d][e];
                shift.activities.resize(max_activity_per_shift);
            }
        }
    }

    void save_file(const string& problem_definition_file, const string& file_name) const;
};



#endif //OUTPUT_DATA_H
