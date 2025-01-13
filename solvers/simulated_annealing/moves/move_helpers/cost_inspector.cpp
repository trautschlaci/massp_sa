#include "cost_inspector.h"
#include "problem_constants.h"


long long int CostInspector::inspect_cover_change(const State& model, bool do_add_cover, int inspected_start,
                                                  int inspected_length, int inspected_task) {
    long long int cost_change = 0;

    for (int i = inspected_start; i < inspected_start + inspected_length; ++i) {
        int current_cover = model.slot_task_cover[i][inspected_task];
        auto& cover_constraints = model.input_data.cover_requirements[i][inspected_task];
        int min_cover = get<0>(cover_constraints);
        int max_cover = get<1>(cover_constraints);

        if (do_add_cover) {
            if(min_cover > current_cover)
                cost_change -= model.cost_weights.understaffing_weight;
            if(current_cover >= max_cover)
                cost_change += (2 * (current_cover - max_cover) + 1);
        }
        else {
            if(min_cover >= current_cover)
                cost_change += model.cost_weights.understaffing_weight;
            if(current_cover > max_cover)
                cost_change -= (2 * (current_cover - max_cover) - 1);
        }
    }

    return cost_change;
}

long long int CostInspector::inspect_consecutive_shift_change(const State& model, int emp, bool shift_is_added, int start) {
    long long int cost_change = 0;

    int day = start / num_time_slots_per_day;

    int shift_count = model.day_emp_shift_count[day][emp];
    if ((shift_is_added && shift_count != 0) || (!shift_is_added && shift_count != 1))
        return 0;

    int inspection_start_day = max(0, day - maximum_consecutive_days);
    int inspection_end_day = min(day, model.input_data.num_periods - maximum_consecutive_days);
    for (int d = inspection_start_day; d < inspection_end_day + 1; ++d) {
        int consecutive_count = 0;
        // Shift is still empty, so 1 should be added to the sum
        if (shift_is_added)
            ++consecutive_count;

        for (int d_index = d; d_index < d + maximum_consecutive_days + 1; ++d_index) {
            if (model.day_emp_shift_count[d_index][emp] > 0)
                ++consecutive_count;
        }

        if (consecutive_count > maximum_consecutive_days)
            cost_change += model.cost_weights.max_consecutive_shift_weight;
    }

    if (!shift_is_added)
        return - cost_change;

    return cost_change;
}

long long int CostInspector::inspect_rest_when_shift_change(const State& model, bool shift_will_be_empty, int period, int emp,
                                                           int shift_start, int shift_end, bool check_prev) {
    long long int cost_change = 0;

    if (period < model.input_data.num_periods - 1) {
        int next_shift = model.period_emp_shifts[period + 1][emp];
        if (model.shift_activity_count[next_shift] > 0) {
            int checked_shift_end = shift_end;
            int shift_day_end = (period + 1) * num_time_slots_per_day;
            if (shift_day_end == shift_start)
                checked_shift_end = max(checked_shift_end, shift_day_end + 40);
            cost_change += model.min_rest_cost_function(model.shift_start[next_shift] - checked_shift_end);
        }
    }
    if (check_prev && (period > 0)) {
        int prev_shift = model.period_emp_shifts[period - 1][emp];
        if (model.shift_activity_count[prev_shift] > 0) {
            int prev_shift_start = model.shift_start[prev_shift];
            int prev_shift_end = prev_shift_start + model.calculate_shift_length(prev_shift, -1);
            int prev_day_end = period * num_time_slots_per_day;
            if (prev_day_end == prev_shift_start)
                prev_shift_end = max(prev_shift_end , prev_day_end + 40);
            cost_change += model.min_rest_cost_function(shift_start - prev_shift_end);
        }
    }

    if (shift_will_be_empty)
        return - cost_change;

    return cost_change;
}

long long int CostInspector::inspect_workload_change(const State& model, int emp, int length_change) {
    auto& workload_constraint = model.input_data.employee_workloads[emp];
    int emp_workload_min = get<0>(workload_constraint);
    int emp_workload_max = get<1>(workload_constraint);
    int current_workload = model.emp_workload[emp];

    long long int last_workload_cost = model.workload_cost_function(current_workload, emp_workload_min,
                                                                    emp_workload_max);
    long long int new_workload_cost = model.workload_cost_function(current_workload + length_change, emp_workload_min,
                                                                   emp_workload_max);

    return new_workload_cost - last_workload_cost;
}
