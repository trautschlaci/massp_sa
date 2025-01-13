#ifndef ACTIVITY_TASK_CHANGE_MOVE_H
#define ACTIVITY_TASK_CHANGE_MOVE_H

#include "../move_base.h"


class ActivityTaskChangeMove : public MoveBase {
    int shift = -1;
    int length = -1;
    int activity = -1;
    int activity_start = -1;
    int new_task = -1;
    int old_task = -1;
    int old_prev_task_length = -1;
    int old_next_task_length = -1;

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //ACTIVITY_TASK_CHANGE_MOVE_H
