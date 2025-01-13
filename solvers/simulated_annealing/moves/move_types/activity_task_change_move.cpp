#include "activity_task_change_move.h"


bool ActivityTaskChangeMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    if (state.input_data.num_tasks == 1)
        return false;

    activity = random_selector.select_random_activity(state, shift);
    if (shift == -1)
        return false;

    activity_start = state.shift_start[shift] + state.calculate_shift_length(shift, activity);
    length = state.activity_length[shift][activity];

    old_task = state.activity_task[shift][activity];
    new_task = random_selector.select_random_task_change(old_task);

    return true;
}

long long int ActivityTaskChangeMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    cost_difference += cost_inspector.inspect_cover_change(state, false, activity_start, length, old_task);
    for(int slot = activity_start; slot < activity_start + length; ++slot)
        --state.slot_task_cover[slot][old_task];
    cost_difference += cost_inspector.inspect_cover_change(state, true, activity_start, length, new_task);

    for(int slot = activity_start; slot < activity_start + length; ++slot)
        ++state.slot_task_cover[slot][old_task];

    int temp_length = length;
    if (activity > 0 && (state.activity_task[shift][activity - 1] == new_task)) {
        auto prev_length = state.activity_length[shift][activity - 1];

        cost_difference -= state.activity_length_cost_function(temp_length);
        cost_difference -= state.activity_length_cost_function(prev_length);

        temp_length += prev_length;
        cost_difference += state.activity_length_cost_function(temp_length);
    }
    if ((activity < state.shift_activity_count[shift] - 1) && (state.activity_task[shift][activity + 1] == new_task))
        {
        auto next_length = state.activity_length[shift][activity + 1];

        cost_difference -= state.activity_length_cost_function(temp_length);
        cost_difference -= state.activity_length_cost_function(next_length);

        temp_length += next_length;
        cost_difference += state.activity_length_cost_function(temp_length);
        }

    return cost_difference;
}

void ActivityTaskChangeMove::abstract_do_move(State& state)
{
    state.activity_task[shift][activity] = new_task;

    for (int slot = activity_start; slot < activity_start + length; ++slot) {
        auto& slot_cover = state.slot_task_cover[slot];
        --slot_cover[old_task];
        ++slot_cover[new_task];
    }

    old_prev_task_length = -1;
    old_next_task_length = -1;
    int activity_reduction = 0;

    int temp_length = length;
    int temp_activity = activity;

    if (temp_activity > 0 && (state.activity_task[shift][temp_activity - 1] == new_task)) {
        --temp_activity;
        old_prev_task_length = state.activity_length[shift][temp_activity];
        temp_length += old_prev_task_length;
        state.activity_length[shift][temp_activity] = temp_length;

        for (int a = temp_activity + 2; a < state.shift_activity_count[shift]; ++a) {
            state.activity_length[shift][a-1] = state.activity_length[shift][a];
            state.activity_task[shift][a-1] = state.activity_task[shift][a];
        }

        --state.shift_activity_count[shift];
        --state.all_activity_count;

        ++activity_reduction;
    }

    if ((temp_activity < state.shift_activity_count[shift] - 1) && (state.activity_task[shift][temp_activity + 1] == new_task)) {
        old_next_task_length = state.activity_length[shift][temp_activity + 1];
        temp_length += old_next_task_length;
        state.activity_length[shift][temp_activity] = temp_length;

        for (int a = temp_activity + 2; a < state.shift_activity_count[shift]; ++a) {
            state.activity_length[shift][a-1] = state.activity_length[shift][a];
            state.activity_task[shift][a-1] = state.activity_task[shift][a];
        }

        --state.shift_activity_count[shift];
        --state.all_activity_count;

        ++activity_reduction;
    }

    if (activity_reduction > 0)
    {
        const int shift_activity_count = state.shift_activity_count[shift];
        state.remove_shift_from_container(shift, shift_activity_count + activity_reduction);
        state.add_shift_to_container(shift, shift_activity_count);
    }
}

void ActivityTaskChangeMove::abstract_revert_move(State& state)
{
    int activity_reduction = (old_prev_task_length != -1) + (old_next_task_length != -1);

    if (activity_reduction > 0)
    {
        const int shift_activity_count = state.shift_activity_count[shift];
        state.remove_shift_from_container(shift, shift_activity_count);
        state.add_shift_to_container(shift, shift_activity_count + activity_reduction);
    }

    int temp_activity = activity;
    if (old_prev_task_length != -1)
        --temp_activity;

    if (old_next_task_length != -1) {
        ++state.all_activity_count;
        ++state.shift_activity_count[shift];

        for (int a = state.shift_activity_count[shift] - 1; a >= temp_activity + 2; --a) {
            state.activity_length[shift][a] = state.activity_length[shift][a-1];
            state.activity_task[shift][a] = state.activity_task[shift][a-1];
        }

        state.activity_length[shift][temp_activity + 1] = old_next_task_length;
    }

    if (old_prev_task_length != -1)
    {
        ++state.all_activity_count;
        ++state.shift_activity_count[shift];

        for (int a = state.shift_activity_count[shift] - 1; a >= temp_activity + 2; --a) {
            state.activity_length[shift][a] = state.activity_length[shift][a-1];
            state.activity_task[shift][a] = state.activity_task[shift][a-1];
        }

        state.activity_length[shift][temp_activity] = old_prev_task_length;
    }

    for (int slot = activity_start; slot < activity_start + length; ++slot) {
        auto& slot_cover = state.slot_task_cover[slot];
        ++slot_cover[old_task];
        --slot_cover[new_task];
    }

    state.activity_task[shift][activity] = old_task;
    state.activity_length[shift][activity] = length;
}
