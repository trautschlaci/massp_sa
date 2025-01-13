#include "output_data.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <problem_constants.h>


pugi::xml_node OutputData::create_solution_tag(pugi::xml_document& doc, const string& problem_definition_dir)
{
    pugi::xml_node solution_data = doc.append_child("Solution");
    solution_data.append_attribute("xmlns:xsi").set_value("http://www.w3.org/2001/XMLSchema-instance");
    solution_data.append_attribute("xsi:noNamespaceSchemaLocation").set_value("Solution.xsd");

    pugi::xml_node problem_element = solution_data.append_child("ProblemFile");
    problem_element.text().set(problem_definition_dir.c_str());

    return solution_data;
}

void OutputData::create_employee_element(pugi::xml_node& solution_data, int emp_id) const {
    pugi::xml_node emp_element = solution_data.append_child("Employee");
    emp_element.append_attribute("ID").set_value(emp_id + 1);

    for (int d = 0; d < num_periods+1; ++d) {
        auto shift = shifts[d][emp_id];

        if (shift.activity_count == 0) {
            continue;
        }

        pugi::xml_node shift_element = emp_element.append_child("Shift");

        pugi::xml_node day_element = shift_element.append_child("Day");

        int start_time = shift.start % num_time_slots_per_day;

        int day = d;
        day_element.text().set(day);

        pugi::xml_node start_element = shift_element.append_child("Start");
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << start_time / 4 << ":" << std::setw(2) << (start_time % 4) * 15 << ":00";
        std::string start_text = ss.str();
        start_element.text().set(start_text.c_str());

        int last_task = shift.activities[0].task;
        int all_activity_length = shift.activities[0].length;
        for (int a = 1; a < shift.activity_count; ++a) {
            int task = shift.activities[a].task;
            int actual_activity_length = shift.activities[a].length;

            if (task != last_task) {
                create_task_element(shift_element, last_task, all_activity_length);
                all_activity_length = actual_activity_length;
            } else {
                all_activity_length += actual_activity_length;
            }

            last_task = task;
        }

        create_task_element(shift_element, last_task, all_activity_length);
    }
}

void OutputData::create_task_element(pugi::xml_node& shift_element, int task, int task_length) {
    pugi::xml_node task_element = shift_element.append_child("Task");
    task_element.append_attribute("WorkLength").set_value(task_length * 15);

    pugi::xml_node id_element = task_element.append_child("ID");
    id_element.text().set(task + 1);

    pugi::xml_node length_element = task_element.append_child("Length");
    length_element.text().set(task_length * 15);
}


void OutputData::save_file(const string& problem_definition_file, const string& file_name) const
{
    pugi::xml_document doc;
    auto solution_data = create_solution_tag(doc, problem_definition_file);

    for (int emp_id = 0; emp_id < num_emps; ++emp_id) {
        create_employee_element(solution_data, emp_id);
    }

    std::ostringstream xml_data;
    solution_data.print(xml_data);
    std::ofstream file(file_name, std::ios::binary);
    file << xml_data.str();
}
