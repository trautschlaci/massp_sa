#ifndef SHIFT_SWAP_MOVE_H
#define SHIFT_SWAP_MOVE_H

#include "../move_base.h"


class ShiftSwapMove : public MoveBase {
    int shift = -1;
    int period = -1;
    int emp = -1;
    int length = -1;
    int shift_start = -1;

    int other_shift = -1;
    int other_emp = -1;
    int other_length = -1;
    int other_shift_start = -1;
    bool is_other_shift_empty = false;

    int length_change = -1;

    int day = -1;
    int other_day = -1;

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //SHIFT_SWAP_MOVE_H
