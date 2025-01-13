#include "shift_swap_period_move.h"
#include "problem_constants.h"


bool ShiftSwapPeriodMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    shift = random_selector.select_random_shift(state, false, true, true);
    if (shift == -1)
        return false;
    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    length = state.calculate_shift_length(shift, -1);
    old_shift_start = state.shift_start[shift];

    other_period = random_selector.select_random_period_change(period);
    other_shift = state.period_emp_shifts[other_period][emp];
    old_other_shift_start = state.shift_start[other_shift];
    is_other_shift_empty = (state.shift_activity_count[other_shift] == 0);
    other_length = is_other_shift_empty ? 0 : state.calculate_shift_length(other_shift, -1);

    new_shift_start = old_shift_start + (other_period - period) * num_time_slots_per_day;
    new_other_shift_start = old_other_shift_start + (period - other_period) * num_time_slots_per_day;

    if (new_shift_start + length > state.input_data.horizon_end)
        return false;
    if (new_other_shift_start + other_length > state.input_data.horizon_end)
        return false;


    old_day = old_shift_start / num_time_slots_per_day;
    new_day = new_shift_start / num_time_slots_per_day;
    old_other_day = old_other_shift_start / num_time_slots_per_day;
    new_other_day = new_other_shift_start / num_time_slots_per_day;

    return true;
}

long long int ShiftSwapPeriodMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;


    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, false, old_shift_start);
    --state.day_emp_shift_count[old_day][emp];

    if (!is_other_shift_empty) {
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, false, old_other_shift_start);
        --state.day_emp_shift_count[old_other_day][emp];
    }

    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, true, new_shift_start);
    ++state.day_emp_shift_count[new_day][emp];

    if (!is_other_shift_empty) {
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, true, new_other_shift_start);
        ++state.day_emp_shift_count[new_other_day][emp];
    }

    --state.day_emp_shift_count[new_day][emp];
    ++state.day_emp_shift_count[old_day][emp];

    if (!is_other_shift_empty) {
        --state.day_emp_shift_count[new_other_day][emp];
        ++state.day_emp_shift_count[old_other_day][emp];
    }


    int temp_shift_activity_count = state.shift_activity_count[shift];
    int temp_other_shift_activity_count = state.shift_activity_count[other_shift];


    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, old_shift_start, old_shift_start + length, true);
    state.shift_activity_count[shift] = 0;

    if (!is_other_shift_empty) {
        cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, other_period, emp, old_other_shift_start, old_other_shift_start + other_length, true);
        state.shift_activity_count[other_shift] = 0;
    }

    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, other_period, emp, new_shift_start, new_shift_start + length, true);
    state.period_emp_shifts[other_period][emp] = shift;
    state.shift_start[shift] = new_shift_start;
    state.shift_activity_count[shift] = temp_shift_activity_count;

    if (!is_other_shift_empty) {
        cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, new_other_shift_start, new_other_shift_start + other_length, true);
        state.period_emp_shifts[period][emp] = other_shift;
        state.shift_start[other_shift] = new_other_shift_start;
        state.shift_activity_count[other_shift] = temp_other_shift_activity_count;
    }


    state.period_emp_shifts[other_period][emp] = other_shift;
    state.shift_start[shift] = old_shift_start;

    if (!is_other_shift_empty) {
        state.period_emp_shifts[period][emp] = shift;
        state.shift_start[other_shift] = old_other_shift_start;
    }


    cost_difference += inspect_and_change_cover(cost_inspector, state, old_shift_start, shift, false, true);
    cost_difference += inspect_and_change_cover(cost_inspector, state, new_shift_start, shift, true, true);
    cost_difference += inspect_and_change_cover(cost_inspector, state, old_other_shift_start, other_shift, false, true);
    cost_difference += inspect_and_change_cover(cost_inspector, state, new_other_shift_start, other_shift, true, true);

    inspect_and_change_cover(cost_inspector, state, old_shift_start, shift, true, false);
    inspect_and_change_cover(cost_inspector, state, new_shift_start, shift, false, false);
    inspect_and_change_cover(cost_inspector, state, old_other_shift_start, other_shift, true, false);
    inspect_and_change_cover(cost_inspector, state, new_other_shift_start, other_shift, false, false);


    return cost_difference;
}

long long int ShiftSwapPeriodMove::inspect_and_change_cover(CostInspector& cost_inspector, State& state, int inspected_start,
        int inspected_shift, bool do_add_cover, bool do_inspect)
{
    long long int cost_difference = 0;
    int cover_start = inspected_start;
    int diff = do_add_cover? 1 : -1;
    for (int activity = 0; activity < state.shift_activity_count[inspected_shift]; ++activity) {
        int activity_length = state.activity_length[inspected_shift][activity];
        int activity_task = state.activity_task[inspected_shift][activity];

        if (do_inspect)
            cost_difference += cost_inspector.inspect_cover_change(state, do_add_cover, cover_start, activity_length, activity_task);
        for(int slot = cover_start; slot < cover_start + activity_length; ++slot)
            state.slot_task_cover[slot][activity_task] += diff;

        cover_start += activity_length;
    }
    return cost_difference;
}

void ShiftSwapPeriodMove::abstract_do_move(State& state)
{
    state.change_shift_cover(-1, shift, old_shift_start, 0);
    state.change_shift_cover(-1, other_shift, old_other_shift_start, 0);


    --state.day_emp_shift_count[old_day][emp];
    ++state.day_emp_shift_count[new_day][emp];

    if (!is_other_shift_empty) {
        --state.day_emp_shift_count[old_other_day][emp];
        ++state.day_emp_shift_count[new_other_day][emp];
    }

    state.period_emp_shifts[period][emp] = other_shift;
    state.period_emp_shifts[other_period][emp] = shift;

    state.shift_period[shift] = other_period;
    state.shift_period[other_shift] = period;

    state.shift_start[shift] = new_shift_start;
    state.shift_start[other_shift] = new_other_shift_start;

    state.change_shift_cover(1, shift, new_shift_start, 0);
    state.change_shift_cover(1, other_shift, new_other_shift_start, 0);
}

void ShiftSwapPeriodMove::abstract_revert_move(State& state)
{
    state.change_shift_cover(-1, other_shift, new_other_shift_start, 0);
    state.change_shift_cover(-1, shift, new_shift_start, 0);

    state.shift_start[other_shift] = old_other_shift_start;
    state.shift_start[shift] = old_shift_start;

    state.shift_period[other_shift] = other_period;
    state.shift_period[shift] = period;

    state.period_emp_shifts[other_period][emp] = other_shift;
    state.period_emp_shifts[period][emp] = shift;


    ++state.day_emp_shift_count[old_day][emp];
    --state.day_emp_shift_count[new_day][emp];

    if (!is_other_shift_empty) {
        ++state.day_emp_shift_count[old_other_day][emp];
        --state.day_emp_shift_count[new_other_day][emp];
    }

    state.change_shift_cover(1, other_shift, old_other_shift_start, 0);
    state.change_shift_cover(1, shift, old_shift_start, 0);
}
