#include <iostream>
#include "utils.h"
#include "IO/input_data.h"
#include "IO/output_data.h"
#include "hyper_parameters.h"
#include "sa_solver.h"
//#include "solvers/mathematical/cp_sat/cp_sat_solver.h"
//#include "solvers/mathematical/cp_sat_linearized/cp_sat_linearized_solver.h"
//#include "solvers/mathematical/gurobi/gurobi_solver.h"
//#include "solvers/mathematical/gurobi_linearized/gurobi_linearized_solver.h"
#include <thread>
#include "json.hpp"

using json = nlohmann::json;


void parse_params(HyperParameters& params, unordered_map<string, string>& args)
{
    if (args.count("max_iterations"))
        params.max_iterations = convert<long long int>(args["max_iterations"]);

    if (args.count("initial_stage_mode"))
        params.initial_stage_mode = convert<int>(args["initial_stage_mode"]);
    if (args.count("solver_algorithm"))
        params.solver_algorithm = convert<int>(args["solver_algorithm"]);

    if (args.count("start_temp"))
        params.start_temp = convert<double>(args["start_temp"]);

    if (args.count("min_temp"))
        params.min_temp = convert<double>(args["min_temp"]);
    if (args.count("cooling_rate"))
        params.cooling_rate = convert<double>(args["cooling_rate"]);
    if (args.count("cutoff_ratio"))
        params.cutoff_ratio = convert<double>(args["cutoff_ratio"]);

    if (args.count("max_activity_per_shift"))
        params.max_activity_per_shift = convert<int>(args["max_activity_per_shift"]);

    if (args.count("shift_add_sampling_weight"))
        params.shift_add_sampling_weight = convert<double>(args["shift_add_sampling_weight"]);
    if (args.count("shift_remove_sampling_weight"))
        params.shift_remove_sampling_weight = convert<double>(args["shift_remove_sampling_weight"]);
    if (args.count("shift_start_change_sampling_weight"))
        params.shift_start_change_sampling_weight = convert<double>(args["shift_start_change_sampling_weight"]);
    if (args.count("shift_swap_sampling_weight"))
        params.shift_swap_sampling_weight = convert<double>(args["shift_swap_sampling_weight"]);
    if (args.count("shift_swap_period_sampling_weight"))
        params.shift_swap_period_sampling_weight = convert<double>(args["shift_swap_period_sampling_weight"]);
    if (args.count("activity_length_change_sampling_weight"))
        params.activity_length_change_sampling_weight = convert<double>(args["activity_length_change_sampling_weight"]);
    if (args.count("activity_task_change_sampling_weight"))
        params.activity_task_change_sampling_weight = convert<double>(args["activity_task_change_sampling_weight"]);
    if (args.count("activity_add_sampling_weight"))
        params.activity_add_sampling_weight = convert<double>(args["activity_add_sampling_weight"]);
    if (args.count("activity_remove_sampling_weight"))
        params.activity_remove_sampling_weight = convert<double>(args["activity_remove_sampling_weight"]);
}


void parse_cost_weights(CostWeights& weights, unordered_map<string, string>& args) {
    if (args.count("max_consecutive_shift_weight"))
        weights.max_consecutive_shift_weight = convert<long long int>(args["max_consecutive_shift_weight"]);
    if (args.count("min_rest_weight"))
        weights.min_rest_weight = convert<long long int>(args["min_rest_weight"]);
    if (args.count("min_workload_weight"))
        weights.min_workload_weight = convert<long long int>(args["min_workload_weight"]);
    if (args.count("max_workload_weight"))
        weights.max_workload_weight = convert<long long int>(args["max_workload_weight"]);
    if (args.count("min_activity_length_weight"))
        weights.min_activity_length_weight = convert<long long int>(args["min_activity_length_weight"]);
    if (args.count("shift_length_weight"))
        weights.shift_length_weight = convert<long long int>(args["shift_length_weight"]);
    if (args.count("understaffing_weight"))
        weights.understaffing_weight = convert<long long int>(args["understaffing_weight"]);
}


void print_solve_info(double run_time, long long int cost)
{
    json sol_dict = {
        {"cost", cost},
        {"run_time", run_time}
    };
    std::cout << sol_dict.dump(2) << std::flush;
}


void run_mathematical_solver()
{
    /*string input_dir = "IO/input/txt";

    auto output_dir = create_output_directory_name();
    create_output_directory(output_dir);

    auto problems = get_problem_files(input_dir);
    if (problems.empty()) {
        cerr << "No problems found in the input directory." << endl;
        return;
    }

    for (int problem_num = 1; problem_num < 225; ++problem_num)
    {
        string problem_string;
        for (const auto & problem : problems)
        {
            if (stoi(problem.substr(0, problem.find('_'))) != problem_num)
                continue;

            problem_string = problem.substr(0, problem.find('.'));
        }

        string problem_definition_loc = "../../input/xml/" + problem_string + ".ros";
        auto input_data = InputData("IO/input/txt/" + problem_string + ".txt");

        auto algorithm = CPSatLinearizedSolver(input_data);
        auto outputData = OutputData(input_data, 10);

        string log_file_name = output_dir + "/" + problem_string + ".log";
        algorithm.solve_problem(outputData, log_file_name, 3600.0);

        string output_file_name = output_dir + "/" + problem_string + ".xml";
        outputData.save_file(problem_definition_loc, output_file_name);
    }*/
}


int main(int argc, char* argv[]) {
    //run_mathematical_solver();

    ///*
    auto args = parse_args(argc, argv);

    string input_dir = "IO/input/txt";

    auto output_dir = args.count("output_dir") ? convert<string>(args["output_dir"]) : create_output_directory_name();
    bool dont_save_solutions = args.count("dont_save_solutions") ? true : false;

    if (!dont_save_solutions)
        create_output_directory(output_dir);

    auto problems = get_problem_files(input_dir);
    if (problems.empty()) {
        cerr << "No problems found in the input directory." << endl;
        return 1;
    }

    auto params = HyperParameters();
    parse_params(params, args);

    auto one_task_params = HyperParameters(params);
    one_task_params.max_activity_per_shift = 1;
    one_task_params.activity_add_sampling_weight = 0;
    one_task_params.activity_remove_sampling_weight = 0;
    one_task_params.activity_task_change_sampling_weight = 0;

    auto costWeights = CostWeights();
    parse_cost_weights(costWeights, args);

    int problem_num = 225;
    if (args.count("instance"))
        problem_num = convert<int>(args["instance"]);

    long long int save_limit_cost = 50000;
    if (args.count("save_limit_cost"))
        save_limit_cost = convert<long long int>(args["save_limit_cost"]);

    int solve_repeat_num = 1;
    if (args.count("solve_repeat_num"))
        solve_repeat_num = convert<int>(args["solve_repeat_num"]);

    string problem_string;
    for (const auto & problem : problems)
    {
        if (stoi(problem.substr(0, problem.find('_'))) != problem_num)
            continue;

        problem_string = problem.substr(0, problem.find('.'));
    }

    string problem_definition_loc = "../../input/xml/" + problem_string + ".ros";
    auto input_data = InputData("IO/input/txt/" + problem_string + ".txt");

    auto algorithm = SASolver(input_data);
    auto outputData = OutputData(input_data, params.max_activity_per_shift);

    const auto& used_params = (input_data.num_tasks == 1) ? one_task_params : params;

    double run_time = 0.0;
    for (int i = 0; i < solve_repeat_num; i++)
        run_time += algorithm.solve_problem(outputData, used_params, costWeights, true, save_limit_cost);
    print_solve_info(run_time, outputData.cost);

    if (!dont_save_solutions)
    {
        unsigned int thread_id = static_cast<unsigned int>(hash<thread::id>{}(this_thread::get_id()));
        string output_file_name = output_dir + "/" + problem_string + "_" + to_string(outputData.cost) + "_" + to_string(thread_id) + ".xml";
        outputData.save_file(problem_definition_loc, output_file_name);
    }
    //*/

    return 0;
}
