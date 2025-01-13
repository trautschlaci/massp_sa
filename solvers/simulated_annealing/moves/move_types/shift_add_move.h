#ifndef SHIFT_ADD_MOVE_H
#define SHIFT_ADD_MOVE_H

#include "../move_base.h"


class ShiftAddMove : public MoveBase {
    int shift = -1;
    int period = -1;
    int emp = -1;
    int new_start = -1;
    int new_length = -1;
    int new_task = -1;

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //SHIFT_ADD_MOVE_H
