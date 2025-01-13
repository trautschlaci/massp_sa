#ifndef HYPER_PARAMETERS_H
#define HYPER_PARAMETERS_H


class CostWeights
{
public:
    long long int max_consecutive_shift_weight;
    long long int min_rest_weight;
    long long int min_workload_weight;
    long long int max_workload_weight;
    long long int min_activity_length_weight;
    long long int shift_length_weight;
    long long int understaffing_weight;

    explicit CostWeights(
        long long int max_consecutive_shift_weight = 570702,
        long long int min_rest_weight = 6603552,
        long long int min_workload_weight = 4900456,
        long long int max_workload_weight = 1929282,
        long long int min_activity_length_weight = 625677,
        long long int shift_length_weight = 8309176,
        long long int understaffing_weight = 5429931
        )
        :
        max_consecutive_shift_weight(max_consecutive_shift_weight),
        min_rest_weight(min_rest_weight),
        min_workload_weight(min_workload_weight),
        max_workload_weight(max_workload_weight),
        min_activity_length_weight(min_activity_length_weight),
        shift_length_weight(shift_length_weight),
        understaffing_weight(understaffing_weight) {}
};


class HyperParameters {
public:
    long long int max_iterations;

    int initial_stage_mode;

    int solver_algorithm;

    double start_temp;

    double min_temp;
    double cooling_rate;
    double cutoff_ratio;

    int max_activity_per_shift;

    double shift_add_sampling_weight;
    double shift_remove_sampling_weight;
    double shift_start_change_sampling_weight;
    double shift_swap_sampling_weight;
    double shift_swap_period_sampling_weight;
    double activity_length_change_sampling_weight;
    double activity_task_change_sampling_weight;
    double activity_add_sampling_weight;
    double activity_remove_sampling_weight;

    explicit HyperParameters(
        long long int max_iterations = 18000000,
        int initial_stage_mode = 1,
        int solver_algorithm = 1,
        double start_temp = 71428600000.0,
        double min_temp = 41015.625,
        double cooling_rate = 0.8573251028806584,
        double cutoff_ratio = 0.10096000000000001,
        int max_activity_per_shift = 6,
        double shift_add_sampling_weight = 0.754458161865569,
        double shift_remove_sampling_weight = 0.6536438767843727,
        double shift_start_change_sampling_weight = 0.6035502958579881,
        double shift_swap_sampling_weight = 0.8512110726643598,
        double shift_swap_period_sampling_weight = 0.17835909631391203,
        double activity_length_change_sampling_weight = 0.8428720083246618,
        double activity_task_change_sampling_weight = 0.9141274238227147,
        double activity_add_sampling_weight = 0.5330812854442344,
        double activity_remove_sampling_weight = 0.0096
        )
        :
        max_iterations(max_iterations),
        initial_stage_mode(initial_stage_mode),
        solver_algorithm(solver_algorithm),
        start_temp(start_temp),
        min_temp(min_temp),
        cooling_rate(cooling_rate),
        cutoff_ratio(cutoff_ratio),
        max_activity_per_shift(max_activity_per_shift),
        shift_add_sampling_weight(shift_add_sampling_weight),
        shift_remove_sampling_weight(shift_remove_sampling_weight),
        shift_start_change_sampling_weight(shift_start_change_sampling_weight),
        shift_swap_sampling_weight(shift_swap_sampling_weight),
        shift_swap_period_sampling_weight(shift_swap_period_sampling_weight),
        activity_length_change_sampling_weight(activity_length_change_sampling_weight),
        activity_task_change_sampling_weight(activity_task_change_sampling_weight),
        activity_add_sampling_weight(activity_add_sampling_weight),
        activity_remove_sampling_weight(activity_remove_sampling_weight)
    {}

    HyperParameters(HyperParameters const& other) = default;
};

#endif //HYPER_PARAMETERS_H
