#include "activity_length_change_move.h"
#include "problem_constants.h"


bool ActivityLengthChangeMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    activity = random_selector.select_random_activity(state, shift);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    shift_start = state.shift_start[shift];
    activity_start = shift_start + state.calculate_shift_length(shift, activity);

    old_length = state.activity_length[shift][activity];
    old_shift_length = state.calculate_shift_length(shift, -1);

    int excluding_shift_length = old_shift_length - old_length;
    int maximum_length =
        min
        (
            state.input_data.horizon_end - (shift_start + excluding_shift_length),
            shift_start + max_shift_length - activity_start
        );

    if (maximum_length <= 0)
        return false;

    int min_length = max(min_shift_length - excluding_shift_length, min_activity_length);

    if (maximum_length <= min_length) {
        new_length = maximum_length;
        if (new_length == old_length)
            return false;
    }
    else {
        if ((min_length == old_length) && (maximum_length == old_length))
            return false;

        if ((old_length >= min_length) && (old_length <= maximum_length)) {
            new_length = random_selector.select_uniform_with_exclusion(min_length, maximum_length, old_length);
        }
        else {
            new_length = random_selector.select_uniform(min_length, maximum_length);
        }
    }

    task = state.activity_task[shift][activity];

    length_change = new_length - old_length;

    return true;
}

long long int ActivityLengthChangeMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    int new_shift_length = old_shift_length + length_change;

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, shift_start, shift_start + old_shift_length, false);
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, shift_start, shift_start + new_shift_length, false);

    cost_difference += cost_inspector.inspect_workload_change(state, emp, length_change);

    cost_difference -= state.activity_length_cost_function(old_length);
    cost_difference += state.activity_length_cost_function(new_length);

    cost_difference -= state.shift_length_cost_function(old_shift_length);
    cost_difference += state.shift_length_cost_function(new_shift_length);


    cost_difference += cost_inspector.inspect_cover_change(state, false, activity_start, old_length, task);
    for(int slot = activity_start; slot < activity_start + old_length; ++slot)
        --state.slot_task_cover[slot][task];
    cost_difference += cost_inspector.inspect_cover_change(state, true, activity_start, new_length, task);
    for(int slot = activity_start; slot < activity_start + new_length; ++slot)
        ++state.slot_task_cover[slot][task];

    int all_activity = state.shift_activity_count[shift];

    int prev_cover_start = activity_start + old_length;
    int new_cover_start = activity_start + new_length;
    for (int a = activity + 1; a < all_activity; ++a) {
        int activity_length = state.activity_length[shift][a];
        int activity_task = state.activity_task[shift][a];

        cost_difference += cost_inspector.inspect_cover_change(state, false, prev_cover_start, activity_length, activity_task);
        for(int slot = prev_cover_start; slot < prev_cover_start + activity_length; ++slot)
            --state.slot_task_cover[slot][activity_task];
        cost_difference += cost_inspector.inspect_cover_change(state, true, new_cover_start, activity_length, activity_task);
        for(int slot = new_cover_start; slot < new_cover_start + activity_length; ++slot)
            ++state.slot_task_cover[slot][activity_task];

        prev_cover_start += activity_length;
        new_cover_start += activity_length;
    }

    for(int slot = activity_start; slot < activity_start + old_length; ++slot)
        ++state.slot_task_cover[slot][task];
    for(int slot = activity_start; slot < activity_start + new_length; ++slot)
        --state.slot_task_cover[slot][task];

    prev_cover_start = activity_start + old_length;
    new_cover_start = activity_start + new_length;
    for (int a = activity + 1; a < all_activity; ++a) {
        int activity_length = state.activity_length[shift][a];
        int activity_task = state.activity_task[shift][a];

        for(int slot = prev_cover_start; slot < prev_cover_start + activity_length; ++slot)
            ++state.slot_task_cover[slot][activity_task];
        for(int slot = new_cover_start; slot < new_cover_start + activity_length; ++slot)
            --state.slot_task_cover[slot][activity_task];

        prev_cover_start += activity_length;
        new_cover_start += activity_length;
    }

    return cost_difference;
}

void ActivityLengthChangeMove::abstract_do_move(State& state)
{
    state.change_shift_cover(-1, shift, activity_start, activity);

    state.activity_length[shift][activity] = new_length;

    state.emp_workload[emp] += length_change;
    state.change_shift_cover(1, shift, activity_start, activity);

    state.shift_length[shift] += length_change;
}

void ActivityLengthChangeMove::abstract_revert_move(State& state)
{
    state.shift_length[shift] -= length_change;

    state.change_shift_cover(-1, shift, activity_start, activity);
    state.emp_workload[emp] -= length_change;

    state.activity_length[shift][activity] = old_length;

    state.change_shift_cover(1, shift, activity_start, activity);
}
