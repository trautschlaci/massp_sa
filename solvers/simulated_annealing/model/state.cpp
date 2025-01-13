#include "state.h"
#include <../IO/problem_constants.h>

using namespace std;


State::State(const InputData& inputData, const CostWeights& costWeights, int maxActivityPerShift)
: input_data(inputData), cost_weights(costWeights), max_activity_per_shift(maxActivityPerShift)
{
    cost = 0;
    int num_periods = inputData.num_periods;
    int num_tasks = inputData.num_tasks;
    int num_emps = inputData.num_emps;

    all_activity_count = 0;

    period_emp_shifts = new int*[num_periods];
    for (int i = 0; i < num_periods; ++i) {
        period_emp_shifts[i] = new int[num_emps];
    }
    empty_shifts = new int[num_periods * num_emps];
    empty_shift_pointers = new int[num_periods * num_emps];
    int count = 0;
    for (int i = 0; i < num_periods; ++i) {
        for (int j = 0; j < num_emps; ++j) {
            period_emp_shifts[i][j] = count;
            empty_shifts[i * num_emps + j] = count;
            empty_shift_pointers[count] = i * num_emps + j;
            ++count;
        }
    }

    day_emp_shift_count = new int*[num_periods + 1];
    for (int i = 0; i < num_periods + 1; ++i) {
        day_emp_shift_count[i] = new int[num_emps]();
    }
    empty_shift_count = num_periods * num_emps;

    one_activity_shifts = new int[num_periods * num_emps];
    at_least_two_activity_non_full_shifts = new int[num_periods * num_emps];
    full_shifts = new int[num_periods * num_emps];

    one_activity_shift_count = 0;
    at_least_two_activity_non_full_shift_count = 0;
    full_shift_count = 0;

    one_activity_shift_pointers = new int[num_periods * num_emps];
    at_least_two_activity_non_full_shift_pointers = new int[num_periods * num_emps];
    full_shift_pointers = new int[num_periods * num_emps];

    shift_period = new int[num_periods * num_emps];
    shift_emp = new int[num_periods * num_emps];
    for (int i = 0; i < num_periods * num_emps; ++i) {
        shift_period[i] = i / num_emps;
        shift_emp[i] = i % num_emps;
    }

    shift_start = new int[num_periods * num_emps]();
    shift_activity_count = new int[num_periods * num_emps]();
    shift_length = new int[num_periods * num_emps]();

    activity_length = new int*[num_periods * num_emps];
    activity_task = new int*[num_periods * num_emps];
    for (int i = 0; i < num_periods * num_emps; ++i) {
        activity_length[i] = new int[maxActivityPerShift];
        activity_task[i] = new int[maxActivityPerShift];
    }

    emp_workload = new int[num_emps]();

    all_slot_num = (num_periods + 1) * 24 * 4;

    slot_task_cover = new int*[all_slot_num];
    for (int i = 0; i < all_slot_num; ++i) {
        slot_task_cover[i] = new int[num_tasks]();
    }
}

State::~State() {
    for (int i = 0; i < input_data.num_periods; ++i) {
        delete[] period_emp_shifts[i];
    }
    delete[] period_emp_shifts;

    for (int i = 0; i < (input_data.num_periods + 1); ++i) {
        delete[] day_emp_shift_count[i];
    }
    delete[] day_emp_shift_count;

    delete[] empty_shifts;
    delete[] empty_shift_pointers;
    delete[] one_activity_shifts;
    delete[] at_least_two_activity_non_full_shifts;
    delete[] full_shifts;
    delete[] one_activity_shift_pointers;
    delete[] at_least_two_activity_non_full_shift_pointers;
    delete[] full_shift_pointers;
    delete[] shift_period;
    delete[] shift_emp;
    delete[] shift_start;
    delete[] shift_activity_count;
    delete[] shift_length;
    delete[] emp_workload;

    for (int i = 0; i < (input_data.num_periods * input_data.num_emps); ++i) {
        delete[] activity_length[i];
        delete[] activity_task[i];
    }
    delete[] activity_length;
    delete[] activity_task;

    for (int i = 0; i < all_slot_num; ++i) {
        delete[] slot_task_cover[i];
    }
    delete[] slot_task_cover;
}


void State::calculate_total_cost() {
    cost = 0;
    calculate_consecutive_days_cost();
    calculate_min_rest_cost();
    calculate_employee_workload_cost();
    calculate_activity_length_cost();
    calculate_shift_length_cost();
    calculate_cover_cost();
}

void State::calculate_consecutive_days_cost() {
    for (int e = 0; e < input_data.num_emps; ++e) {
        for (int d = maximum_consecutive_days; d < input_data.num_periods + 1; ++d) {
            int consecutive_count = 0;
            for (int d_index = d-maximum_consecutive_days; d_index < d + 1; ++d_index) {
                int shift_count = day_emp_shift_count[d_index][e];
                if (shift_count > 0)
                    ++consecutive_count;
            }

            if (consecutive_count > maximum_consecutive_days)
                cost += cost_weights.max_consecutive_shift_weight;
        }
    }
}

void State::calculate_min_rest_cost() {
    for (int e = 0; e < input_data.num_emps; ++e) {
        for (int d = 1; d < input_data.num_periods; ++d) {
            int shift_index = period_emp_shifts[d][e];
            int prev_shift_index = period_emp_shifts[d-1][e];

            if ((shift_activity_count[shift_index] > 0) && (shift_activity_count[prev_shift_index] > 0)) {
                int sft_start = shift_start[shift_index];
                int prev_sft_start = shift_start[prev_shift_index];

                int prev_shift_end = prev_sft_start + calculate_shift_length(prev_shift_index, -1);
                if (d * num_time_slots_per_day == prev_sft_start)
                    prev_shift_end = max(prev_shift_end, d * num_time_slots_per_day + 10 * 4);

                cost += min_rest_cost_function(sft_start - prev_shift_end);
            }
        }
    }
}

void State::calculate_employee_workload_cost() {
    for (int e = 0; e < input_data.num_emps; ++e) {
        auto& workload_constraint = input_data.employee_workloads[e];
        cost += workload_cost_function(emp_workload[e], get<0>(workload_constraint), get<1>(workload_constraint));
    }
}

void State::calculate_activity_length_cost() {
    for (int e = 0; e < input_data.num_emps; ++e) {
        for (int d = 0; d < input_data.num_periods; ++d) {
            int shift_index = period_emp_shifts[d][e];

            for(int a = 0; a < shift_activity_count[shift_index]; ++a)
                cost += activity_length_cost_function(activity_length[shift_index][a]);
        }
    }
}

void State::calculate_shift_length_cost() {
    for (int e = 0; e < input_data.num_emps; ++e) {
        for (int d = 0; d < input_data.num_periods; ++d) {
            int shift_index = period_emp_shifts[d][e];

            if (shift_activity_count[shift_index] > 0) {
                cost += shift_length_cost_function(calculate_shift_length(shift_index, -1));
            }
        }
    }
}

void State::calculate_cover_cost() {
    for (int s = 0; s < all_slot_num; ++s) {
        for (int t = 0; t < input_data.num_tasks; ++t) {
            int actual_staff = slot_task_cover[s][t];
            int min_staff = get<0>(input_data.cover_requirements[s][t]);
            int max_staff = get<1>(input_data.cover_requirements[s][t]);
            cost += objective_function(actual_staff, min_staff, max_staff);
        }
    }
}


int State::calculate_shift_length(int shift_id, int before_activity) const
{
    if (before_activity == -1)
        return shift_length[shift_id];

    int length = 0;
    for (int i = 0; i < before_activity; ++i)
        length += activity_length[shift_id][i];
    return length;
}

void State::add_shift_to_container(int* shift_pointers, int* shifts, int& shift_count, int shift_index) {
    shifts[shift_count] = shift_index;
    shift_pointers[shift_index] = shift_count;
    ++shift_count;
}

void State::remove_shift_from_container(int* shift_pointers, int* shifts, int& shift_count, int shift_index) {
    if (shift_count > 1) {
        int shift_reverse_index =  shift_pointers[shift_index];
        int last_element = shifts[shift_count - 1];
        shifts[shift_reverse_index] = last_element;
        shift_pointers[last_element] = shift_reverse_index;
        --shift_count;
    }
    else {
        --shift_count;
    }
}

void State::add_shift_to_container(int shift_index, int activity_count)
{
    if (activity_count == 0)
        add_shift_to_container(empty_shift_pointers, empty_shifts, empty_shift_count, shift_index);
    else if (activity_count == max_activity_per_shift)
        add_shift_to_container(full_shift_pointers, full_shifts, full_shift_count, shift_index);
    else if (activity_count == 1)
        add_shift_to_container(one_activity_shift_pointers, one_activity_shifts, one_activity_shift_count, shift_index);
    else
        add_shift_to_container(at_least_two_activity_non_full_shift_pointers, at_least_two_activity_non_full_shifts, at_least_two_activity_non_full_shift_count, shift_index);
}

void State::remove_shift_from_container(int shift_index, int activity_count)
{
    if (activity_count == 0)
        remove_shift_from_container(empty_shift_pointers, empty_shifts, empty_shift_count, shift_index);
    else if (activity_count == max_activity_per_shift)
        remove_shift_from_container(full_shift_pointers, full_shifts, full_shift_count, shift_index);
    else if (activity_count == 1)
        remove_shift_from_container(one_activity_shift_pointers, one_activity_shifts, one_activity_shift_count, shift_index);
    else
        remove_shift_from_container(at_least_two_activity_non_full_shift_pointers, at_least_two_activity_non_full_shifts, at_least_two_activity_non_full_shift_count, shift_index);
}

int State::get_shift_count_of_size(int min_size, int max_size) const
{
    if (min_size > max_size)
        return 0;

    int count = 0;

    if (min_size <= 0)
        count += empty_shift_count;
    if (max_size >= max_activity_per_shift)
        count += full_shift_count;
    if (min_size <= 1 && max_size >= 1 && 1 != max_activity_per_shift)
        count += one_activity_shift_count;
    if (min_size <= 2 && max_size >= 2 && 2 < max_activity_per_shift)
        count += at_least_two_activity_non_full_shift_count;

    return count;
}

int State::get_nth_shift_of_size(int min_size, int max_size, int n_shift) const
{
    int select = n_shift;

    if (min_size <= 0)
    {
        if (select < empty_shift_count)
            return empty_shifts[select];

        select -= empty_shift_count;
    }

    if (max_size >= max_activity_per_shift)
    {
        if (select < full_shift_count)
            return full_shifts[select];

        select -= full_shift_count;
    }

    if (min_size <= 1 && max_size >= 1 && 1 != max_activity_per_shift)
    {
        if (select < one_activity_shift_count)
            return one_activity_shifts[select];

        select -= one_activity_shift_count;
    }

    if (min_size <= 2 && max_size >= 2 && 2 < max_activity_per_shift)
    {
        if (select < at_least_two_activity_non_full_shift_count)
            return at_least_two_activity_non_full_shifts[select];
    }

    return -1;
}

int State::get_nth_activity(int n_activity, int& selected_shift_id) const
{
    if (n_activity >= full_shift_count * max_activity_per_shift) {
        n_activity -= full_shift_count * max_activity_per_shift;

        if (n_activity >= one_activity_shift_count) {
            n_activity -= one_activity_shift_count;

            for (int i = 0; i < at_least_two_activity_non_full_shift_count; ++i) {
                int activity_count = shift_activity_count[at_least_two_activity_non_full_shifts[i]];

                if (n_activity < activity_count) {
                    selected_shift_id = at_least_two_activity_non_full_shifts[i];
                    return n_activity;
                }

                n_activity -= activity_count;
            }
        }

        selected_shift_id = one_activity_shifts[n_activity];
        return 0;
    }

    selected_shift_id = full_shifts[n_activity / max_activity_per_shift];
    return n_activity % max_activity_per_shift;
}


long long int State::min_rest_cost_function(int actual_rest_length) const
{
    if (actual_rest_length < min_rest_length)
        return (min_rest_length - actual_rest_length) * cost_weights.min_rest_weight;
    return 0;
}

long long int State::workload_cost_function(int actual_workload, int min_workload, int max_workload) const
{
    if (actual_workload < min_workload)
        return (min_workload - actual_workload) * cost_weights.min_workload_weight;
    if (actual_workload > max_workload)
        return (actual_workload - max_workload) * cost_weights.max_workload_weight;
    return 0;
}

long long int State::activity_length_cost_function(int actual_activity_length) const
{
    if (actual_activity_length < min_activity_length)
        return (min_activity_length - actual_activity_length) * cost_weights.min_activity_length_weight;
    return 0;
}

long long int State::shift_length_cost_function(int actual_length) const
{
    if (actual_length < min_shift_length) {
        int diff = min_shift_length - actual_length;
        return cost_weights.shift_length_weight * diff * diff;
    }
    if (actual_length > max_shift_length) {
        int diff = actual_length - max_shift_length;
        return cost_weights.shift_length_weight * diff * diff;
    }
    return 0;
}

long long int State::objective_function(int actual_staff, int min_staff, int max_staff) const
{
    if (actual_staff < min_staff)
        return (min_staff - actual_staff) * cost_weights.understaffing_weight;
    if (actual_staff > max_staff) {
        int diff = actual_staff - max_staff;
        return diff * diff;
    }
    return 0;
}

void State::change_shift_cover(int added_number, int inspected_shift, int activity_start, int activity_start_idx)
{
    for (int i = activity_start_idx; i < shift_activity_count[inspected_shift]; ++i) {
        int length = activity_length[inspected_shift][i];
        int task = activity_task[inspected_shift][i];

        for (int slot = activity_start; slot < activity_start + length; ++slot)
            slot_task_cover[slot][task] += added_number;

        activity_start += length;
    }
}

void State::add_activity(int selected_emp, int selected_period, int selected_shift_start, int selected_task, int selected_activity_length)
{
    int shift = period_emp_shifts[selected_period][selected_emp];

    int& selected_shift_activity_count = shift_activity_count[shift];
    if (selected_shift_activity_count == 0)
    {
        ++day_emp_shift_count[selected_shift_start / num_time_slots_per_day][selected_emp];
        shift_start[shift] = selected_shift_start;
        shift_length[shift] = 0;
    }

    activity_length[shift][selected_shift_activity_count] = selected_activity_length;
    activity_task[shift][selected_shift_activity_count] = selected_task;
    ++selected_shift_activity_count;

    change_shift_cover(1, shift, selected_shift_start + shift_length[shift], selected_shift_activity_count-1);

    shift_length[shift] += selected_activity_length;
    ++all_activity_count;

    emp_workload[selected_emp] += selected_activity_length;

    remove_shift_from_container(shift, selected_shift_activity_count-1);
    add_shift_to_container(shift, selected_shift_activity_count);
}
