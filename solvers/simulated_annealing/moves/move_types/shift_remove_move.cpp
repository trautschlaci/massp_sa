#include "shift_remove_move.h"
#include "problem_constants.h"


bool ShiftRemoveMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    shift = random_selector.select_random_shift(state, false, true, true);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    old_start = state.shift_start[shift];
    old_activity_num = state.shift_activity_count[shift];
    old_length = state.calculate_shift_length(shift, -1);

    return true;
}

long long int ShiftRemoveMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, false, old_start);

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, old_start, old_start + old_length, true);

    cost_difference += cost_inspector.inspect_workload_change(state, emp, - old_length);

    cost_difference -= state.shift_length_cost_function(old_length);

    int cover_start = old_start;
    for (int activity = 0; activity < old_activity_num; ++activity) {
        int activity_length = state.activity_length[shift][activity];

        cost_difference -= state.activity_length_cost_function(activity_length);

        cost_difference += cost_inspector.inspect_cover_change(state, false, cover_start, activity_length, state.activity_task[shift][activity]);

        cover_start += activity_length;
    }

    return cost_difference;
}

void ShiftRemoveMove::abstract_do_move(State& state)
{
    --state.day_emp_shift_count[old_start / num_time_slots_per_day][emp];

    state.emp_workload[emp] -= old_length;

    state.change_shift_cover(-1, shift, old_start, 0);

    state.shift_activity_count[shift] = 0;

    state.remove_shift_from_container(shift, old_activity_num);
    state.add_shift_to_container(shift, 0);

    state.all_activity_count -= old_activity_num;
}

void ShiftRemoveMove::abstract_revert_move(State& state)
{
    state.all_activity_count += old_activity_num;

    state.remove_shift_from_container(shift, 0);
    state.add_shift_to_container(shift, old_activity_num);

    state.shift_activity_count[shift] = old_activity_num;

    state.change_shift_cover(1, shift, old_start, 0);

    state.emp_workload[emp] += old_length;

    ++state.day_emp_shift_count[old_start / num_time_slots_per_day][emp];
}
