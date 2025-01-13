#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "input_data.h"
#include "output_data.h"
#include "moves/move_base.h"
#include "model/state.h"


class StateManager {
    const InputData& input_data;
    OutputData& best_solution;
    RandomSelector& random_selector;

    long long int save_limit_cost;

    State create_initial_state(CostWeights& costWeights, int maxActivityPerShift, int initial_stage_mode);
    void populate_initial_state_greedily(State& initial_state, int maxActivityPerShift);

    void transform_state_to_best_output(const State& state) const;
public:
    State current_state;

    StateManager(InputData& inputData, OutputData& solution, RandomSelector& random_selector,
        CostWeights& costWeights, int maxActivityPerShift, int initial_stage_mode, long long int save_limit_cost):
    input_data(inputData), best_solution(solution), random_selector(random_selector), save_limit_cost(save_limit_cost),
    current_state(create_initial_state(costWeights, maxActivityPerShift, initial_stage_mode))
    {}

    bool create_move(MoveBase* moveType) const;
    long long int delta_inspect_move(CostInspector& cost_inspector, MoveBase* moveType);
    long long int full_inspect_move(MoveBase* moveType);
    void do_move(MoveBase* moveType);
    void revert_move(MoveBase* moveType);
};


#endif //STATE_MANAGER_H
