#ifndef STATE_H
#define STATE_H

#include <vector>
#include <../IO/input_data.h>
#include "hyper_parameters.h"

using namespace std;


class State {
    void calculate_consecutive_days_cost();
    void calculate_min_rest_cost();
    void calculate_employee_workload_cost();
    void calculate_activity_length_cost();
    void calculate_shift_length_cost();
    void calculate_cover_cost();


    static void add_shift_to_container(int* shift_pointers, int* shifts, int& shift_count, int shift_index);
    static void remove_shift_from_container(int* shift_pointers, int* shifts, int& shift_count, int shift_index);

    int* empty_shifts;
    int* one_activity_shifts;
    int* at_least_two_activity_non_full_shifts;
    int* full_shifts;

    int empty_shift_count;
    int one_activity_shift_count;
    int at_least_two_activity_non_full_shift_count;
    int full_shift_count;

    int* empty_shift_pointers;
    int* one_activity_shift_pointers;
    int* at_least_two_activity_non_full_shift_pointers;
    int* full_shift_pointers;

public:
    long long int cost;

    int** period_emp_shifts;
    int** day_emp_shift_count;
    int* shift_activity_count;
    int* shift_period;
    int* shift_emp;
    int* shift_start;
    int* shift_length;
    int** activity_length;
    int** activity_task;

    int all_activity_count;

    int* emp_workload;
    int** slot_task_cover;

    int all_slot_num;


    const InputData& input_data;
    const CostWeights& cost_weights;
    int max_activity_per_shift;


    State(const InputData& inputData, const CostWeights& costWeights, int maxActivityPerShift);
    ~State();


    long long int min_rest_cost_function(int actual_rest_length) const;
    long long int workload_cost_function(int actual_workload, int min_workload, int max_workload) const;
    long long int activity_length_cost_function(int actual_activity_length) const;
    long long int shift_length_cost_function(int actual_length) const;
    long long int objective_function(int actual_staff, int min_staff, int max_staff) const;

    void calculate_total_cost();


    int calculate_shift_length(int shift_id, int before_activity) const;

    int get_shift_count_of_size(int min_size, int max_size) const;
    int get_nth_shift_of_size(int min_size, int max_size, int n_shift) const;
    int get_nth_activity(int n_activity, int& selected_shift_id) const;


    void add_shift_to_container(int shift_index, int activity_count);
    void remove_shift_from_container(int shift_index, int activity_count);

    void change_shift_cover(int added_number, int inspected_shift, int activity_start, int activity_start_idx);
    void add_activity(int selected_emp, int selected_period, int selected_shift_start, int selected_task, int selected_activity_length);
};



#endif //STATE_H
