#include "shift_swap_move.h"
#include "problem_constants.h"


bool ShiftSwapMove::create_random_move(RandomSelector& random_selector, const State& state)
{
    shift = random_selector.select_random_shift(state, false, true, true);
    if (shift == -1)
        return false;
    period = state.shift_period[shift];
    emp = state.shift_emp[shift];
    length = state.calculate_shift_length(shift, -1);
    shift_start = state.shift_start[shift];

    other_emp = random_selector.select_random_emp_change(emp);
    other_shift = state.period_emp_shifts[period][other_emp];
    other_shift_start = state.shift_start[other_shift];
    is_other_shift_empty = (state.shift_activity_count[other_shift] == 0);
    other_length = is_other_shift_empty? 0 : state.calculate_shift_length(other_shift, -1);

    length_change = other_length - length;

    day = shift_start / num_time_slots_per_day;
    other_day = other_shift_start / num_time_slots_per_day;

    return true;
}

long long int ShiftSwapMove::delta_inspect_move(CostInspector& cost_inspector, State& state)
{
    cost_difference = 0;


    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, false, shift_start);
    --state.day_emp_shift_count[day][emp];

    if (!is_other_shift_empty) {
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, other_emp, false, other_shift_start);
        --state.day_emp_shift_count[other_day][other_emp];
    }

    cost_difference += cost_inspector.inspect_consecutive_shift_change(state, other_emp, true, shift_start);

    if (!is_other_shift_empty)
        cost_difference += cost_inspector.inspect_consecutive_shift_change(state, emp, true, other_shift_start);


    ++state.day_emp_shift_count[day][emp];

    if (!is_other_shift_empty)
        ++state.day_emp_shift_count[other_day][other_emp];


    int shift_end = shift_start + length;
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, emp, shift_start, shift_end, true);
    cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, other_emp, shift_start, shift_end, true);

    if (!is_other_shift_empty) {
        int other_shift_end = other_shift_start + other_length;
        cost_difference += cost_inspector.inspect_rest_when_shift_change(state, false, period, emp, other_shift_start, other_shift_end, true);
        cost_difference += cost_inspector.inspect_rest_when_shift_change(state, true, period, other_emp, other_shift_start, other_shift_end, true);
    }


    cost_difference += cost_inspector.inspect_workload_change(state, emp, length_change);
    cost_difference += cost_inspector.inspect_workload_change(state, other_emp, - length_change);

    return cost_difference;
}

void ShiftSwapMove::abstract_do_move(State& state)
{
    auto& day_shift_count = state.day_emp_shift_count[day];
    --day_shift_count[emp];
    ++day_shift_count[other_emp];

    if (!is_other_shift_empty) {
        auto& other_day_shift_count = state.day_emp_shift_count[other_day];
        --other_day_shift_count[other_emp];
        ++other_day_shift_count[emp];
    }

    state.period_emp_shifts[period][emp] = other_shift;
    state.period_emp_shifts[period][other_emp] = shift;

    state.emp_workload[emp] += length_change;
    state.emp_workload[other_emp] -= length_change;

    state.shift_emp[shift] = other_emp;
    state.shift_emp[other_shift] = emp;
}

void ShiftSwapMove::abstract_revert_move(State& state)
{
    state.shift_emp[other_shift] = other_emp;
    state.shift_emp[shift] = emp;

    state.emp_workload[other_emp] += length_change;
    state.emp_workload[emp] -= length_change;

    state.period_emp_shifts[period][other_emp] = other_shift;
    state.period_emp_shifts[period][emp] = shift;

    auto& day_shift_count = state.day_emp_shift_count[day];
    ++day_shift_count[emp];
    --day_shift_count[other_emp];

    if (!is_other_shift_empty) {
        auto& other_day_shift_count = state.day_emp_shift_count[other_day];
        ++other_day_shift_count[other_emp];
        --other_day_shift_count[emp];
    }
}
