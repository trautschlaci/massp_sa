#ifndef COST_INSPECTOR_H
#define COST_INSPECTOR_H

#include "model/state.h"


class CostInspector {
public:
    static long long int inspect_cover_change(const State& model, bool do_add_cover, int inspected_start,
        int inspected_length, int inspected_task);
    static long long int inspect_consecutive_shift_change(const State& model, int emp, bool shift_is_added, int start);
    static long long int inspect_rest_when_shift_change(const State& model, bool shift_will_be_empty, int period, int emp,
                                                 int shift_start, int shift_end, bool check_prev);
    static long long int inspect_workload_change(const State& model, int emp, int length_change);
};


#endif //COST_INSPECTOR_H
