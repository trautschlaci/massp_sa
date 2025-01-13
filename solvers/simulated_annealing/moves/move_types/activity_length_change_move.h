#ifndef ACTIVITY_LENGTH_CHANGE_MOVE_H
#define ACTIVITY_LENGTH_CHANGE_MOVE_H

#include "../move_base.h"


class ActivityLengthChangeMove : public MoveBase {
    int shift = -1;
    int activity = -1;
    int period = -1;
    int emp = -1;
    int task = -1;
    int shift_start = -1;
    int activity_start = -1;
    int new_length = -1;
    int old_length = -1;
    int old_shift_length = -1;
    int length_change = -1;

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //ACTIVITY_LENGTH_CHANGE_MOVE_H
