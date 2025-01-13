#include "fast_sa_algorithm.h"
#include <cmath>
#include <random>


SA_Algorithm::SA_Algorithm(std::mt19937& randomMachine, double startTemp, long long int maxIterations)
: random_machine(randomMachine) {
    this->dist = std::uniform_real_distribution<double>(0.0, 1.0);
    this->start_temp = startTemp;
    this->max_iterations = maxIterations;
    this->next_iteration = 1;
}


double SA_Algorithm::temperature() const
{
    return start_temp / static_cast<double>(next_iteration);
}

double SA_Algorithm::metropolis(const long long int& cost_delta, double temperature) {
    return exp(static_cast<double>(-cost_delta) / temperature);
}


bool SA_Algorithm::is_finished() const
{
    return next_iteration > max_iterations;
}

void SA_Algorithm::tick(bool was_accepted)
{
    ++next_iteration;
}

bool SA_Algorithm::do_accept(const long long int& cost_delta) {
    if (cost_delta <= 0)
        return true;

    return metropolis(cost_delta, temperature()) >= dist(random_machine);
}
