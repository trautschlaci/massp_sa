#ifndef SA_SOLVER_H
#define SA_SOLVER_H

#include <random>
#include "input_data.h"
#include "output_data.h"
#include "hyper_parameters.h"
#include "moves/move_base.h"
#include "moves/move_types/activity_add_move.h"
#include "moves/move_types/activity_length_change_move.h"
#include "moves/move_types/activity_remove_move.h"
#include "moves/move_types/activity_task_change_move.h"
#include "moves/move_types/shift_swap_period_move.h"
#include "moves/move_types/shift_add_move.h"
#include "moves/move_types/shift_remove_move.h"
#include "moves/move_types/shift_start_change_move.h"
#include "moves/move_types/shift_swap_move.h"

using namespace std;


class SASolver {
    InputData& input_data;

    vector<unique_ptr<MoveBase>> selectable_moves;

    MoveBase* select_random_move_type(mt19937& random_machine, discrete_distribution<>& dist) const;

public:
    explicit SASolver(InputData& inputData) : input_data(inputData)
    {
        selectable_moves.push_back(std::make_unique<ShiftAddMove>());
        selectable_moves.push_back(std::make_unique<ShiftRemoveMove>());
        selectable_moves.push_back(std::make_unique<ShiftStartChangeMove>());
        selectable_moves.push_back(std::make_unique<ShiftSwapMove>());
        selectable_moves.push_back(std::make_unique<ShiftSwapPeriodMove>());
        selectable_moves.push_back(std::make_unique<ActivityLengthChangeMove>());
        selectable_moves.push_back(std::make_unique<ActivityTaskChangeMove>());
        selectable_moves.push_back(std::make_unique<ActivityAddMove>());
        selectable_moves.push_back(std::make_unique<ActivityRemoveMove>());
    }

    double solve_problem(OutputData& outputData, const HyperParameters& hyperParams, CostWeights& cost_weights,
        bool use_delta_inspection, long long int save_limit_cost) const;
};


#endif //SA_SOLVER_H
