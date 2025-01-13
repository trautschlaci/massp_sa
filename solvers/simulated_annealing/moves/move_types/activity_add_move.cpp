#include "activity_add_move.h"
#include "problem_constants.h"


bool ActivityAddMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    if (state.input_data.num_tasks == 1)
        return false;

    shift = random_selector.select_random_shift(state, false, true, false);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    old_activity_num = state.shift_activity_count[shift];
    shift_start = state.shift_start[shift];
    old_shift_length = state.calculate_shift_length(shift, -1);

    activity_start = shift_start + old_shift_length;
    int maximum_length = min(state.input_data.horizon_end - activity_start, max_shift_length - old_shift_length);

    if (maximum_length <= 0)
        return false;

    if (maximum_length <= min_activity_length)
        new_length = maximum_length;
    else
        new_length = random_selector.select_uniform(min_activity_length, maximum_length);

    new_task = random_selector.select_random_task_change(state.activity_task[shift][old_activity_num-1]);

    return true;
}

long long int ActivityAddMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, shift_start, activity_start, false);
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, shift_start, activity_start + new_length, false);

    cost_difference += cost_inspector.inspect_workload_change(state, emp, new_length);

    cost_difference += state.activity_length_cost_function(new_length);

    cost_difference -= state.shift_length_cost_function(old_shift_length);
    cost_difference += state.shift_length_cost_function(old_shift_length + new_length);

    cost_difference += cost_inspector.inspect_cover_change(state, true, activity_start, new_length, new_task);

    return cost_difference;
}

void ActivityAddMove::abstract_do_move(State& state)
{
    ++state.shift_activity_count[shift];
    state.activity_length[shift][old_activity_num] = new_length;
    state.activity_task[shift][old_activity_num] = new_task;

    state.emp_workload[emp] += new_length;
    state.change_shift_cover(1, shift, activity_start, old_activity_num);

    if (old_activity_num == 1 || old_activity_num == state.max_activity_per_shift - 1) {
        state.remove_shift_from_container(shift, old_activity_num);
        state.add_shift_to_container(shift, old_activity_num + 1);
    }

    ++state.all_activity_count;

    state.shift_length[shift] += new_length;
}

void ActivityAddMove::abstract_revert_move(State& state)
{
    state.shift_length[shift] -= new_length;

    --state.all_activity_count;

    if (old_activity_num == 1 || old_activity_num == state.max_activity_per_shift - 1) {
        state.remove_shift_from_container(shift, old_activity_num + 1);
        state.add_shift_to_container(shift, old_activity_num);
    }

    state.change_shift_cover(-1, shift, activity_start, old_activity_num);
    state.emp_workload[emp] -= new_length;

    --state.shift_activity_count[shift];
}
