#include "state_manager.h"
#include "problem_constants.h"


bool StateManager::create_move(MoveBase* moveType) const
{
    return moveType->create_random_move(random_selector, current_state);
}

long long int StateManager::delta_inspect_move(CostInspector& cost_inspector, MoveBase* moveType)
{
    return moveType->delta_inspect_move(cost_inspector, current_state);
}

long long int StateManager::full_inspect_move(MoveBase* moveType)
{
    const long long int cost_difference = moveType->full_inspect_move(current_state);

    if (best_solution.cost > current_state.cost)
    {
        if (save_limit_cost > current_state.cost)
            transform_state_to_best_output(current_state);
        best_solution.cost = current_state.cost;
    }

    return cost_difference;
}

void StateManager::do_move(MoveBase* moveType)
{
    moveType->do_move(current_state);

    if (best_solution.cost > current_state.cost)
    {
        if (save_limit_cost > current_state.cost)
            transform_state_to_best_output(current_state);
        best_solution.cost = current_state.cost;
    }
}

void StateManager::revert_move(MoveBase* moveType)
{
    moveType->revert_move(current_state);
}


State StateManager::create_initial_state(CostWeights& costWeights, int maxActivityPerShift, int initial_stage_mode)
{
    auto new_state = State(input_data, costWeights, maxActivityPerShift);

    if (initial_stage_mode == 1)
    {
        populate_initial_state_greedily(new_state, maxActivityPerShift);
    }

    new_state.calculate_total_cost();
    if (best_solution.cost == -1)
    {
        transform_state_to_best_output(new_state);
        best_solution.cost = new_state.cost;
    }
    return new_state;
}


void StateManager::populate_initial_state_greedily(State& initial_state, int maxActivityPerShift)
{
    for (int s = 0; s < initial_state.all_slot_num; ++s) {
        for (int t = 0; t < input_data.num_tasks; ++t) {
            int actual_staff = initial_state.slot_task_cover[s][t];
            int min_staff = get<0>(input_data.cover_requirements[s][t]);
            int staff_diff = min_staff - actual_staff;

            for (int i = 0; i < staff_diff; ++i) {
                int period = -1;
                int selected_start = -1;
                bool was_found = true;
                int selected_emp = random_selector.select_best_available_emp_and_start(initial_state, s, period, selected_start, was_found);
                if (!was_found)
                    continue;

                int max_selected_length = min(max_shift_length, get<1>(input_data.employee_workloads[selected_emp]) - initial_state.emp_workload[selected_emp]);
                int min_selected_length = max(min_shift_length, selected_start-s);
                int max_end = selected_start + max_selected_length;

                int activity_count = 0;
                bool is_overwork = false;
                for (int j = selected_start; j < max_end && activity_count < maxActivityPerShift; j++) {
                    int selected_task = -1;
                    if (j < s)
                    {
                        selected_task = t;
                    }
                    else
                    {
                        selected_task = random_selector.select_most_needed_task(initial_state, j, is_overwork);
                        if ((j >= selected_start + min_selected_length) && is_overwork)
                        {
                            break;
                        }
                    }

                    int needed_activity_length = 0;
                    for (int l = j; l < max_end; ++l)
                    {
                        if (get<0>(input_data.cover_requirements[l][selected_task]) - initial_state.slot_task_cover[l][selected_task] <= 0)
                        {
                            break;
                        }
                        ++needed_activity_length;
                    }

                    int selected_activity_length = max(min_activity_length, needed_activity_length);
                    selected_activity_length = max(selected_activity_length, s - j);
                    selected_activity_length = min(selected_activity_length, max_end - j);
                    selected_activity_length = max(selected_activity_length, selected_start + min_selected_length - j);

                    if (selected_activity_length < min_activity_length)
                        continue;

                    initial_state.add_activity(selected_emp, period, selected_start, selected_task, selected_activity_length);
                    ++activity_count;
                    j += selected_activity_length - 1;
                }
            }
        }
    }
}


void StateManager::transform_state_to_best_output(const State& state) const
{
    for (int e = 0; e < input_data.num_emps; ++e)
        for (int d = 0; d < input_data.num_periods+1; ++d)
            best_solution.shifts[d][e].activity_count = 0;

    for (int d = 0; d < input_data.num_periods; ++d)
    {
        for (int e = 0; e < input_data.num_emps; ++e)
        {
            int day = d;
            int shift_id = state.period_emp_shifts[d][e];

            int start_time = state.shift_start[shift_id] % num_time_slots_per_day;
            if (start_time == 0)
                ++day;

            auto& shift = best_solution.shifts[day][e];

            int activity_count = state.shift_activity_count[shift_id];

            if (activity_count == 0)
                continue;

            shift.activity_count = activity_count;
            shift.start = state.shift_start[shift_id];

            for (int a = 0; a < shift.activity_count; ++a)
            {
                auto& activity = shift.activities[a];
                activity.length = state.activity_length[shift_id][a];
                activity.task = state.activity_task[shift_id][a];
            }
        }
    }
}
