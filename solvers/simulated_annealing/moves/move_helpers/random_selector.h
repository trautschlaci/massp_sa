#ifndef RANDOM_SELECTOR_H
#define RANDOM_SELECTOR_H

#include <random>
#include "model/state.h"

using namespace std;


class RandomSelector {
    mt19937& random_machine;

    int max_activity_per_shift;
    int num_periods;
    int num_tasks;
    int num_emps;
    vector<int> valid_shift_start_array;

    vector<uniform_int_distribution<>> dist_directory;

    int period_dist_size;
    int task_dist_size;
    int emp_dist_size;

    static bool is_min_rest_respected_before(const State& data_model, int emp, int period, int start);
    static bool is_consecutive_days_respected(const State& data_model, int emp, int start);

public:
    int select_random_shift(const State& data_model, bool only_empty_shifts, bool can_be_single_activity, bool can_be_full);
    int select_random_shift_start(const State& data_model, int period, int minimum_shift_length, int exclude_start);
    int select_random_activity(const State& data_model, int& selected_shift_id);
    int select_random_task();
    int select_random_task_change(int old_task);
    int select_random_emp_change(int old_emp);
    int select_random_period_change(int old_period);

    int select_best_available_emp_and_start(const State& data_model, int inspected_slot, int& selected_period, int& selected_start, bool& was_found);
    static int select_most_needed_task(const State& data_model, int inspected_slot, bool& is_overwork);

    int select_uniform(int max_num);
    int select_uniform(int min_num, int max_num);
    int select_uniform_with_exclusion(int max_num, int exclude_num);
    int select_uniform_with_exclusion(int min_num, int max_num, int exclude_num);

    explicit RandomSelector(mt19937& random_machine, int max_activity_per_shift, int num_periods, int num_tasks, int num_emps);
};


#endif //RANDOM_SELECTOR_H
