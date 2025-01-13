#include "sa_solver.h"
#include <heuristics//fast_sa_algorithm.h>
#include <chrono>
#include "heuristics/cutoff_sa_algorithm.h"
#include "manager/state_manager.h"
#include "model/state.h"
#include "moves/move_enum.h"
#include <thread>



double SASolver::solve_problem(OutputData& outputData, const HyperParameters& hyperParams, CostWeights& cost_weights,
                             bool use_delta_inspection, long long int save_limit_cost) const {
    auto t1_start = chrono::high_resolution_clock::now();

    random_device rd;
    seed_seq seed{rd(), static_cast<unsigned int>(hash<thread::id>{}(this_thread::get_id()))};
    mt19937 mt(seed);

    std::vector<double> sampling_weights =
    {
        hyperParams.shift_add_sampling_weight, hyperParams.shift_remove_sampling_weight, hyperParams.shift_start_change_sampling_weight,
        hyperParams.shift_swap_sampling_weight, hyperParams.shift_swap_period_sampling_weight,
        hyperParams.activity_length_change_sampling_weight, hyperParams.activity_task_change_sampling_weight,
        hyperParams.activity_add_sampling_weight, hyperParams.activity_remove_sampling_weight
    };

    discrete_distribution<> move_dist(sampling_weights.begin(), sampling_weights.end());

    auto random_selector = RandomSelector(mt, hyperParams.max_activity_per_shift, input_data.num_periods, input_data.num_tasks, input_data.num_emps);
    auto cost_inspector = CostInspector();

    auto state_manager = StateManager(input_data, outputData, random_selector, cost_weights,
        hyperParams.max_activity_per_shift, hyperParams.initial_stage_mode, save_limit_cost);


    unique_ptr<SABase> sa_algorithm;
    if (hyperParams.solver_algorithm == 0)
        sa_algorithm = std::make_unique<SA_Algorithm>(mt, hyperParams.start_temp, hyperParams.max_iterations);
    else
        sa_algorithm = std::make_unique<Cutoff_SA_Algorithm>(mt, hyperParams.start_temp, hyperParams.max_iterations, hyperParams.min_temp,
            hyperParams.cooling_rate, hyperParams.cutoff_ratio);


    while (!sa_algorithm->is_finished()) {
        auto move_type = select_random_move_type(mt, move_dist);

        if (move_type == nullptr || !state_manager.create_move(move_type))
            sa_algorithm->tick(false);
        else
        {
            long long int cost_delta = 0;
            if (use_delta_inspection)
                cost_delta = state_manager.delta_inspect_move(cost_inspector, move_type);
            else
                cost_delta = state_manager.full_inspect_move(move_type);

            bool do_accept = sa_algorithm->do_accept(cost_delta);
            if (do_accept) {
                if (use_delta_inspection)
                    state_manager.do_move(move_type);
            }
            else {
                if(!use_delta_inspection)
                    state_manager.revert_move(move_type);
            }

            sa_algorithm->tick(do_accept);
        }
    }


    auto t1_stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(t1_stop - t1_start);
    double run_time = (double)duration.count() / 1000;

    return run_time;
}


MoveBase* SASolver::select_random_move_type(mt19937& random_machine, discrete_distribution<>& dist) const {
    auto move_type = static_cast<MoveEnum>(dist(random_machine));

    if (move_type == SHIFT_ADD_MOVE)
        return selectable_moves[0].get();
    if (move_type == SHIFT_REMOVE_MOVE)
        return selectable_moves[1].get();
    if (move_type == SHIFT_START_CHANGE_MOVE)
        return selectable_moves[2].get();
    if (move_type == SHIFT_SWAP_MOVE)
        return selectable_moves[3].get();
    if (move_type == SHIFT_SWAP_PERIOD_MOVE)
        return selectable_moves[4].get();
    if (move_type == ACTIVITY_LENGTH_CHANGE_MOVE)
        return selectable_moves[5].get();
    if (move_type == ACTIVITY_TASK_CHANGE_MOVE)
        return selectable_moves[6].get();
    if (move_type == ACTIVITY_ADD_MOVE)
        return selectable_moves[7].get();
    if (move_type == ACTIVITY_REMOVE_MOVE)
        return selectable_moves[8].get();

    return nullptr;
}