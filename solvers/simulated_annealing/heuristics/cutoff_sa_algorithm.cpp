#include "cutoff_sa_algorithm.h"


Cutoff_SA_Algorithm::Cutoff_SA_Algorithm(std::mt19937& randomMachine, double startTemp, long long int maxIterations,
    double minTemp, double coolingRate, double cutoffRatio) : random_machine(randomMachine)
{
    this->cooling_rate = coolingRate;
    this->max_iterations = maxIterations;
    this->current_iteration = 0;
    this->num_accepted = 0;
    this->num_sampled = 0;
    this->current_temp = startTemp;

    this->max_sampled = 0;
    this->max_accepted = 0;

    calculate_params(startTemp, minTemp, cutoffRatio);
}


void Cutoff_SA_Algorithm::calculate_params(double startTemp, double minTemp, double cutoffRatio)
{
    max_sampled = static_cast<long long int>(static_cast<double>(max_iterations) * -log(cooling_rate) / log(startTemp/minTemp));
    max_accepted = static_cast<long long int>(static_cast<double>(max_sampled) * cutoffRatio);
}

double Cutoff_SA_Algorithm::temperature() const
{
    return current_temp;
}

double Cutoff_SA_Algorithm::metropolis(const long long int& cost_delta, double temperature) {
    return exp(static_cast<double>(-cost_delta) / temperature);
}

void Cutoff_SA_Algorithm::tick_temperature()
{
    current_temp *= cooling_rate;
    num_accepted = 0;
    num_sampled = 0;
}



bool Cutoff_SA_Algorithm::is_finished() const
{
    return current_iteration >= max_iterations;
}

void Cutoff_SA_Algorithm::tick(bool was_accepted)
{
    ++current_iteration;
    ++num_sampled;
    if (was_accepted)
        ++num_accepted;

    if (num_accepted > max_accepted || num_sampled > max_sampled)
        tick_temperature();
}

bool Cutoff_SA_Algorithm::do_accept(const long long int& cost_delta) {
    if (cost_delta <= 0)
        return true;

    return metropolis(cost_delta, temperature()) >= dist(random_machine);
}