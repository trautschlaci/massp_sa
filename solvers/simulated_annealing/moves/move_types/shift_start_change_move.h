#ifndef SHIFT_START_CHANGE_MOVE_H
#define SHIFT_START_CHANGE_MOVE_H

#include "../move_base.h"


class ShiftStartChangeMove : public MoveBase {
    int shift = -1;
    int period = -1;
    int emp = -1;
    int new_start = -1;
    int old_start = -1;
    int length = -1;
    int activity_num = -1;
    int old_day = -1;
    int new_day = -1;

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //SHIFT_START_CHANGE_MOVE_H
