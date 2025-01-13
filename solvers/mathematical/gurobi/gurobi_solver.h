#ifndef GUROBI_SOLVER_H
#define GUROBI_SOLVER_H

#include "input_data.h"
#include "output_data.h"


class GurobiSolver {
    const InputData& input_data;
public:
    explicit GurobiSolver(const InputData& inputData) : input_data(inputData) {}

    void solve_problem(OutputData& outputData, const std::string& log_file_name, double time_limit);
};


#endif //GUROBI_SOLVER_H
