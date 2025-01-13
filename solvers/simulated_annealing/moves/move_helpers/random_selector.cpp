#include "random_selector.h"
#include "problem_constants.h"


RandomSelector::RandomSelector(mt19937& random_machine, int max_activity_per_shift, int num_periods, int num_tasks, int num_emps)
: random_machine(random_machine), max_activity_per_shift(max_activity_per_shift), num_periods(num_periods), num_tasks(num_tasks), num_emps(num_emps)
{
    for (auto& valid_starts : valid_shift_starts)
        for (int start = valid_starts[0]; start <= valid_starts[1]; ++start)
            valid_shift_start_array.emplace_back(start);

    dist_directory.resize(num_periods * num_emps * max_activity_per_shift);
    for (int i = 0; i < num_periods * num_emps * max_activity_per_shift; ++i)
    {
        dist_directory[i] = uniform_int_distribution<>(0, i);
    }

    period_dist_size = num_periods - 1;
    task_dist_size = num_tasks - 1;
    emp_dist_size = num_emps - 1;
}


int RandomSelector::select_random_shift(const State& data_model, bool only_empty_shifts, bool can_be_single_activity, bool can_be_full)
{
    int min_activity_count = 0;
    int max_activity_count = 0;

    if (!only_empty_shifts) {
        if (can_be_single_activity) {
            if (can_be_full) {
                min_activity_count = 1;
                max_activity_count = max_activity_per_shift;
            }
            else {
                min_activity_count = 1;
                max_activity_count = max_activity_per_shift - 1;
            }
        }
        else {
            if (can_be_full) {
                min_activity_count = 2;
                max_activity_count = max_activity_per_shift;
            }
            else {
                min_activity_count = 2;
                max_activity_count = max_activity_per_shift - 1;
            }
        }
    }

    int selection_size = data_model.get_shift_count_of_size(min_activity_count, max_activity_count);
    if (selection_size < 1)
        return -1;

    int select = select_uniform(selection_size - 1);

    return data_model.get_nth_shift_of_size(min_activity_count, max_activity_count, select);
}

int RandomSelector::select_random_shift_start(const State& data_model, int period, int minimum_shift_length, int exclude_start){
    int start_index;

    int end_cut_off = data_model.input_data.horizon_end - num_time_slots_per_day * period - minimum_shift_length;

    auto lower = lower_bound(valid_shift_start_array.begin(), valid_shift_start_array.end(), end_cut_off);
    auto end_cut_off_index = distance(valid_shift_start_array.begin(), lower);
    if (end_cut_off_index == valid_shift_start_array.size() || valid_shift_start_array[end_cut_off_index] > end_cut_off)
        --end_cut_off_index;

    if (end_cut_off_index < 1 && (valid_shift_start_array[0] > end_cut_off || valid_shift_start_array[0] == exclude_start))
        return -1;

    if (exclude_start == -1 || exclude_start > end_cut_off) {
        start_index = select_uniform(static_cast<int>(end_cut_off_index));
    }
    else {
        start_index = select_uniform(static_cast<int>(end_cut_off_index) - 1);
        if (valid_shift_start_array[start_index] == exclude_start)
            ++start_index;
    }

    return valid_shift_start_array[start_index] + period * num_time_slots_per_day;
}

int RandomSelector::select_random_activity(const State& data_model, int& selected_shift_id)
{
    if (data_model.all_activity_count == 0) {
        selected_shift_id = -1;
        return -1;
    }

    int activity_index = select_uniform(data_model.all_activity_count - 1);

    return data_model.get_nth_activity(activity_index, selected_shift_id);
}

int RandomSelector::select_random_task()
{
    return select_uniform(task_dist_size);
}

int RandomSelector::select_random_task_change(int old_task)
{
    return select_uniform_with_exclusion(task_dist_size, old_task);
}

int RandomSelector::select_random_emp_change(int old_emp)
{
    return select_uniform_with_exclusion(emp_dist_size, old_emp);
}

int RandomSelector::select_random_period_change(int old_period)
{
    return select_uniform_with_exclusion(period_dist_size, old_period);
}


int RandomSelector::select_best_available_emp_and_start(const State& data_model, int inspected_slot, int& selected_period, int& selected_start, bool& was_found)
{
    selected_period = inspected_slot / num_time_slots_per_day;
    int slot_in_day = inspected_slot % num_time_slots_per_day;
    if (slot_in_day == 0)
    {
        --selected_period;
        slot_in_day = 24*4;
    }

    auto lower = lower_bound(valid_shift_start_array.begin(), valid_shift_start_array.end(), slot_in_day);
    auto start_index = distance(valid_shift_start_array.begin(), lower);
    if (valid_shift_start_array[start_index] != slot_in_day)
        --start_index;

    if (start_index < 0)
    {
        --selected_period;
        start_index = static_cast<int>(valid_shift_start_array.size()) - 1;
    }

    if (selected_period < 0)
    {
        selected_start = -1;
        was_found = false;
        return -1;
    }

    selected_start = valid_shift_start_array[start_index] + selected_period * num_time_slots_per_day;


    int best_selection = -1;
    int workload_needed = 0;
    int available_plus_workload = 0;
    for (int e = 0; e < num_emps; ++e)
    {
        int actual_shift = data_model.period_emp_shifts[selected_period][e];
        if (data_model.shift_activity_count[actual_shift] != 0)
            continue;

        if (selected_period < data_model.input_data.num_periods-1 && data_model.shift_activity_count[data_model.period_emp_shifts[selected_period+1][e]] != 0)
            continue;

        if (!is_min_rest_respected_before(data_model, e, selected_period, selected_start))
            continue;

        if (!is_consecutive_days_respected(data_model, e, selected_start))
            continue;

        int current_workload = data_model.emp_workload[e];
        int min_workload = get<0>(data_model.input_data.employee_workloads[e]);
        int max_workload = get<1>(data_model.input_data.employee_workloads[e]);

        int temp_workload_needed = max(min_workload - current_workload, 0);
        int temp_available_plus_workload = max_workload - current_workload;

        if (temp_workload_needed > workload_needed || (temp_workload_needed == workload_needed && temp_available_plus_workload > available_plus_workload))
        {
            best_selection = e;
            workload_needed = temp_workload_needed;
            available_plus_workload = temp_available_plus_workload;
        }
    }

    if (best_selection == -1)
    {
        was_found = false;
        return -1;
    }

    was_found = true;
    return best_selection;
}

int RandomSelector::select_most_needed_task(const State& data_model, int inspected_slot, bool& is_overwork)
{
    int best_selection = -1;
    int cover_needed = 0;
    int available_plus_cover = 0;

    for (int t = 0; t < data_model.input_data.num_tasks; ++t)
    {
        int current_cover = data_model.slot_task_cover[inspected_slot][t];
        auto& cover_constraints = data_model.input_data.cover_requirements[inspected_slot][t];
        int min_cover = get<0>(cover_constraints);
        int max_cover = get<1>(cover_constraints);

        int temp_cover_needed = max(min_cover - current_cover, 0);
        int temp_available_plus_cover = max_cover - current_cover;

        if (temp_cover_needed > cover_needed || (temp_cover_needed == cover_needed && temp_available_plus_cover > available_plus_cover))
        {
            best_selection = t;
            cover_needed = temp_cover_needed;
            available_plus_cover = temp_available_plus_cover;
        }
    }

    is_overwork = (cover_needed == 0);
    return best_selection;
}


int RandomSelector::select_uniform(int max_num)
{
    return dist_directory[max_num](random_machine);
}

int RandomSelector::select_uniform(int min_num, int max_num)
{
    return min_num + select_uniform(max_num-min_num);
}

int RandomSelector::select_uniform_with_exclusion(int max_num, int exclude_num)
{
    int selection = select_uniform(max_num-1);
    if (selection >= exclude_num)
        ++selection;
    return selection;
}

int RandomSelector::select_uniform_with_exclusion(int min_num, int max_num, int exclude_num)
{
    return min_num + select_uniform_with_exclusion(max_num-min_num, exclude_num-min_num);
}

bool RandomSelector::is_min_rest_respected_before(const State& data_model, int emp, int period, int start)
{
    if (period > 0)
    {
        int prev_shift = data_model.period_emp_shifts[period-1][emp];
        int prev_shift_start = data_model.shift_start[prev_shift];

        int prev_shift_end = prev_shift_start + data_model.calculate_shift_length(prev_shift, -1);
        if (period * num_time_slots_per_day == prev_shift_start)
            prev_shift_end = max(prev_shift_end, period * num_time_slots_per_day + 10 * 4);

        if ((data_model.shift_activity_count[prev_shift] > 0) && (prev_shift_end + 14*4 > start))
            return false;
    }
    return true;
}

bool RandomSelector::is_consecutive_days_respected(const State& data_model, int emp, int start)
{
    int calculated_day = start / num_time_slots_per_day;
    int consecutive_count = 0;

    if ((data_model.day_emp_shift_count[calculated_day][emp] == 0))
        ++consecutive_count;

    int inspection_start_day = max(0, calculated_day - maximum_consecutive_days);
    for (int d = calculated_day - 1; d >= inspection_start_day; --d) {
        if (data_model.day_emp_shift_count[d][emp] > 0)
            ++consecutive_count;
        else
            break;
    }

    int inspection_end_day = min(calculated_day + maximum_consecutive_days, data_model.input_data.num_periods);
    for (int d = calculated_day+1; d <= inspection_end_day; ++d) {
        if (data_model.day_emp_shift_count[d][emp] > 0)
            ++consecutive_count;
        else
            break;
    }

    return consecutive_count <= maximum_consecutive_days;
}
