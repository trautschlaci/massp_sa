#ifndef MOVE_BASE_H
#define MOVE_BASE_H

#include "model/state.h"
#include "move_helpers/cost_inspector.h"
#include "move_helpers/random_selector.h"


class MoveBase {
protected:
    long long int cost_difference = 0;
    virtual void abstract_do_move(State& state) = 0;
    virtual void abstract_revert_move(State& state) = 0;
public:
    virtual ~MoveBase() = default;

    virtual bool create_random_move(RandomSelector& random_selector, const State& state) = 0;

    virtual long long int delta_inspect_move(CostInspector& cost_inspector, State& state) = 0;

    long long int full_inspect_move(State& state)
    {
        const long long int prev_cost = state.cost;
        cost_difference = 0;
        do_move(state);
        state.calculate_total_cost();
        cost_difference = state.cost - prev_cost;
        return cost_difference;
    }

    void do_move(State& state)
    {
        abstract_do_move(state);
        state.cost += cost_difference;
    }

    void revert_move(State& state)
    {
        abstract_revert_move(state);
        state.cost -= cost_difference;
    }
};


#endif //MOVE_BASE_H
