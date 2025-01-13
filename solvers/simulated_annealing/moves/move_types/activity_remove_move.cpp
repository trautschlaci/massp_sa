#include "activity_remove_move.h"


bool ActivityRemoveMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    if (state.input_data.num_tasks == 1)
        return false;

    shift = random_selector.select_random_shift(state, false, false, true);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    new_activity_num = state.shift_activity_count[shift] - 1;
    shift_start = state.shift_start[shift];
    old_activity_length = state.activity_length[shift][new_activity_num];
    new_shift_end = shift_start + state.calculate_shift_length(shift, -1) - old_activity_length;

    return true;
}

long long int ActivityRemoveMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, shift_start, new_shift_end + old_activity_length, false);
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, shift_start, new_shift_end, false);

    cost_difference += cost_inspector.inspect_workload_change(state, emp, - old_activity_length);

    cost_difference -= state.activity_length_cost_function(old_activity_length);

    int new_shift_length = new_shift_end - shift_start;
    cost_difference -= state.shift_length_cost_function(new_shift_length + old_activity_length);
    cost_difference += state.shift_length_cost_function(new_shift_length);

    cost_difference += cost_inspector.inspect_cover_change(state, false, new_shift_end, old_activity_length, state.activity_task[shift][new_activity_num]);

    return cost_difference;
}

void ActivityRemoveMove::abstract_do_move(State& state)
{
    state.change_shift_cover(-1, shift, new_shift_end, new_activity_num);

    --state.shift_activity_count[shift];

    state.emp_workload[emp] -= old_activity_length;

    if (new_activity_num == 1 || new_activity_num == state.max_activity_per_shift - 1) {
        state.remove_shift_from_container(shift, new_activity_num + 1);
        state.add_shift_to_container(shift, new_activity_num);
    }

    --state.all_activity_count;

    state.shift_length[shift] -= old_activity_length;
}

void ActivityRemoveMove::abstract_revert_move(State& state)
{
    state.shift_length[shift] += old_activity_length;

    ++state.all_activity_count;

    if (new_activity_num == 1 || new_activity_num == state.max_activity_per_shift - 1) {
        state.remove_shift_from_container(shift, new_activity_num);
        state.add_shift_to_container(shift, new_activity_num + 1);
    }

    state.emp_workload[emp] += old_activity_length;

    ++state.shift_activity_count[shift];

    state.change_shift_cover(1, shift, new_shift_end, new_activity_num);
}
