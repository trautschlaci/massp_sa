#ifndef SHIFT_SWAP_PERIOD_MOVE_H
#define SHIFT_SWAP_PERIOD_MOVE_H

#include "../move_base.h"


class ShiftSwapPeriodMove : public MoveBase {
    int shift = -1;
    int period = -1;
    int emp = -1;
    int length = -1;
    int old_shift_start = -1;
    int new_shift_start = -1;

    int other_shift = -1;
    int other_period = -1;
    int other_emp = -1;
    int other_length = -1;
    int old_other_shift_start = -1;
    int new_other_shift_start = -1;
    bool is_other_shift_empty = false;

    int old_day = -1;
    int new_day = -1;
    int old_other_day = -1;
    int new_other_day = -1;

    static long long int inspect_and_change_cover(CostInspector& cost_inspector, State& state, int inspected_start,
        int inspected_shift, bool do_add_cover, bool do_inspect);

    void abstract_do_move(State& state) override;
    void abstract_revert_move(State& state) override;
public:
    bool create_random_move(RandomSelector& random_selector, const State& state) override;
    long long int delta_inspect_move(CostInspector& cost_inspector, State& state) override;
};


#endif //SHIFT_SWAP_PERIOD_MOVE_H
