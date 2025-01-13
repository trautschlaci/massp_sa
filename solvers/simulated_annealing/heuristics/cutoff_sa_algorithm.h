#ifndef CUTOFF_SA_ALGORITHM_H
#define CUTOFF_SA_ALGORITHM_H

#include <random>
#include "sa_base.h"


class Cutoff_SA_Algorithm : public SABase {
    std::mt19937& random_machine;
    std::uniform_real_distribution<double> dist;

    double cooling_rate;

    long long int max_iterations;

    long long int max_accepted;
    long long int max_sampled;

    long long int current_iteration;
    long long int num_accepted;
    long long int num_sampled;
    double current_temp;


    void calculate_params(double startTemp, double minTemp, double cutoffRatio);

    double temperature() const;
    static double metropolis(const long long int& cost_delta, double temperature);

    void tick_temperature();

public:
    explicit Cutoff_SA_Algorithm(std::mt19937& randomMachine, double startTemp, long long int maxIterations, double minTemp,
        double coolingRate, double cutoffRatio);

    bool is_finished() const override;
    bool do_accept(const long long int& cost_delta) override;
    void tick(bool was_accepted) override;
};



#endif //CUTOFF_SA_ALGORITHM_H
