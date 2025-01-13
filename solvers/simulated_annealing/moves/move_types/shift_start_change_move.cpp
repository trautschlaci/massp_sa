#include "shift_start_change_move.h"
#include "problem_constants.h"


bool ShiftStartChangeMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    shift = random_selector.select_random_shift(state, false, true, true);
    if (shift == -1)
        return false;

    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    activity_num = state.shift_activity_count[shift];
    length = state.calculate_shift_length(shift, -1);

    old_start = state.shift_start[shift];
    new_start = random_selector.select_random_shift_start(state, period, length, old_start);
    if (new_start == -1)
        return false;

    old_day = old_start / num_time_slots_per_day;
    new_day = new_start / num_time_slots_per_day;

    return true;
}

long long int ShiftStartChangeMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;

    if (old_day != new_day) {
        auto& prev_shift_count = state.day_emp_shift_count[old_day][emp];
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, false, old_start);
        --prev_shift_count;
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, true, new_start);
        ++prev_shift_count;
    }

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, old_start, old_start + length, true);
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, new_start, new_start + length, true);

    int prev_cover_start = old_start;
    int new_cover_start = new_start;
    for (int activity = 0; activity < activity_num; ++activity) {
        int activity_length = state.activity_length[shift][activity];
        int activity_task = state.activity_task[shift][activity];

        cost_difference += cost_inspector.inspect_cover_change(state, false, prev_cover_start, activity_length, activity_task);
        for(int slot = prev_cover_start; slot < prev_cover_start + activity_length; ++slot)
            --state.slot_task_cover[slot][activity_task];

        cost_difference += cost_inspector.inspect_cover_change(state, true, new_cover_start, activity_length, activity_task);
        for(int slot = new_cover_start; slot < new_cover_start + activity_length; ++slot)
            ++state.slot_task_cover[slot][activity_task];

        prev_cover_start += activity_length;
        new_cover_start += activity_length;
    }

    prev_cover_start = old_start;
    new_cover_start = new_start;
    for (int activity = 0; activity < activity_num; ++activity) {
        int activity_length = state.activity_length[shift][activity];
        int activity_task = state.activity_task[shift][activity];

        for(int slot = prev_cover_start; slot < prev_cover_start + activity_length; ++slot)
            ++state.slot_task_cover[slot][activity_task];
        for(int slot = new_cover_start; slot < new_cover_start + activity_length; ++slot)
            --state.slot_task_cover[slot][activity_task];

        prev_cover_start += activity_length;
        new_cover_start += activity_length;
    }

    return cost_difference;
}

void ShiftStartChangeMove::abstract_do_move(State& state)
{
    --state.day_emp_shift_count[old_day][emp];
    ++state.day_emp_shift_count[new_day][emp];

    state.change_shift_cover(-1, shift, old_start, 0);
    state.shift_start[shift] = new_start;
    state.change_shift_cover(1, shift, new_start, 0);
}

void ShiftStartChangeMove::abstract_revert_move(State& state)
{
    state.change_shift_cover(-1, shift, new_start, 0);
    state.shift_start[shift] = old_start;
    state.change_shift_cover(1, shift, old_start, 0);

    ++state.day_emp_shift_count[old_day][emp];
    --state.day_emp_shift_count[new_day][emp];
}