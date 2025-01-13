#ifndef FAST_SA_ALGORITHM_H
#define FAST_SA_ALGORITHM_H

#include <random>
#include "sa_base.h"


class SA_Algorithm : public SABase{
    std::mt19937& random_machine;
    std::uniform_real_distribution<double> dist;

    double start_temp;
    long long int max_iterations;

    long long int next_iteration;

    double temperature() const;
    static double metropolis(const long long int& cost_delta, double temperature);
public:
    explicit SA_Algorithm(std::mt19937& randomMachine, double startTemp, long long int maxIterations);

    bool is_finished() const override;
    bool do_accept(const long long int& cost_delta) override;
    void tick(bool was_accepted) override;
};


#endif //FAST_SA_ALGORITHM_H
