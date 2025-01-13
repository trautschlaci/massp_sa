#include "shift_add_move.h"
#include "problem_constants.h"


bool ShiftAddMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    shift = random_selector.select_random_shift(state, true, false, false);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];

    new_start = random_selector.select_random_shift_start(state, period, min_shift_length , -1);
    if (new_start == -1)
        return false;

    int maximum_length = min(state.input_data.horizon_end - new_start, max_shift_length);
    if (maximum_length < min_shift_length)
        return false;
    new_length = random_selector.select_uniform(min_shift_length, maximum_length);

    new_task = random_selector.select_random_task();

    return true;
}

long long int ShiftAddMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, true, new_start);

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, new_start, new_start + new_length, true);

    cost_difference += cost_inspector.inspect_workload_change(state, emp, new_length);

    cost_difference += cost_inspector.inspect_cover_change(state, true, new_start, new_length, new_task);

    return cost_difference;
}

void ShiftAddMove::abstract_do_move(State& state)
{
    ++state.day_emp_shift_count[new_start / num_time_slots_per_day][emp];

    state.shift_start[shift] = new_start;
    state.shift_activity_count[shift] = 1;
    state.activity_length[shift][0] = new_length;
    state.activity_task[shift][0] = new_task;

    state.emp_workload[emp] += new_length;
    state.change_shift_cover(1, shift, new_start, 0);

    state.remove_shift_from_container(shift, 0);
    state.add_shift_to_container(shift, 1);

    ++state.all_activity_count;

    state.shift_length[shift] = new_length;
}

void ShiftAddMove::abstract_revert_move(State& state)
{
    --state.all_activity_count;

    state.remove_shift_from_container(shift, 1);
    state.add_shift_to_container(shift, 0);

    state.change_shift_cover(-1, shift, new_start, 0);
    state.emp_workload[emp] -= new_length;

    state.shift_activity_count[shift] = 0;

    --state.day_emp_shift_count[new_start / num_time_slots_per_day][emp];
}
