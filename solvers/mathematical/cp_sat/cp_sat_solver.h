#ifndef CP_SAT_SOLVER_H
#define CP_SAT_SOLVER_H

#include "input_data.h"
#include "output_data.h"


class CPSatSolver {
    const InputData& input_data;
public:
    explicit CPSatSolver(const InputData& inputData) : input_data(inputData) {}

    void solve_problem(OutputData& outputData, const std::string& log_file_name, double time_limit) const;
};



#endif //CP_SAT_SOLVER_H
