#include "cp_sat_linearized_solver.h"
#include "ortools/sat/cp_model.h"
#include "../utils.h"
#include <cstdio>

using namespace operations_research;
using namespace sat;



void CPSatLinearizedSolver::solve_problem(OutputData& outputData, const std::string& log_file_name, double time_limit) const
{
    CpModelBuilder cp_model;

    int num_horizon_intervals = input_data.num_periods*96;
    int num_all_intervals = num_horizon_intervals + 96;


    vector<vector<int64_t>> sd(input_data.num_periods+1, vector<int64_t>(0));
    auto vec_1 = create_range(6*4, 10*4);
    auto vec_2 = create_range(14*4, 18*4);
    auto vec_3 = create_range(20*4, 23*4+3);
    sd[0] = {0};
    sd[0].insert(sd[0].end(), vec_1.begin(), vec_1.end());
    sd[0].insert(sd[0].end(), vec_2.begin(), vec_2.end());
    sd[0].insert(sd[0].end(), vec_3.begin(), vec_3.end());
    for (int d = 1; d < input_data.num_periods; d++) {
        int base = d * 96;
        sd[d] = {0, base};
        vec_1 = create_range(base + 6 * 4, base + 10 * 4),
        vec_2 = create_range(base + 14 * 4, base + 18 * 4),
        vec_3 = create_range(base + 20 * 4, base + 23 * 4 + 3);
        sd[d].insert(sd[d].end(), vec_1.begin(), vec_1.end());
        sd[d].insert(sd[d].end(), vec_2.begin(), vec_2.end());
        sd[d].insert(sd[d].end(), vec_3.begin(), vec_3.end());
    }
    sd[input_data.num_periods] = {0, input_data.num_periods*96};


    vector<int64_t> L_set = {0};
    vec_1 = create_range(24, 40);
    L_set.insert(L_set.end(), vec_1.begin(), vec_1.end());


    vector<vector<vector<BoolVar>>> xeia(input_data.num_emps,
                                          vector<vector<BoolVar>>(num_horizon_intervals,
                                                  vector<BoolVar>(input_data.num_tasks)));
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int i = 0; i < num_horizon_intervals; i++) {
            for (int t = 0; t < input_data.num_tasks; t++) {
                xeia[e][i][t] = cp_model.NewBoolVar();
            }
        }
    }

    vector<vector<IntVar>> ted(input_data.num_emps,
                                          vector<IntVar>(input_data.num_periods+1));
    for (int t = 0; t < input_data.num_emps; t++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            ted[t][d] = cp_model.NewIntVar(Domain::FromValues(sd[d]));
        }
    }

    vector<vector<IntVar>> ned(input_data.num_emps,
                                          vector<IntVar>(input_data.num_periods+1));
    for (int t = 0; t < input_data.num_emps; t++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            ned[t][d] = cp_model.NewIntVar(Domain::FromValues(L_set));
        }
    }

    vector<vector<IntVar>> ued(input_data.num_emps,
                                  vector<IntVar>(input_data.num_periods+1));
    for (int t = 0; t < input_data.num_emps; t++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            ued[t][d] = cp_model.NewIntVar({0, num_all_intervals});
        }
    }

    vector<vector<BoolVar>> zed(input_data.num_emps,
                          vector<BoolVar>(input_data.num_periods+1));
    for (int t = 0; t < input_data.num_emps; t++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            zed[t][d] = cp_model.NewBoolVar();
        }
    }

    vector<vector<BoolVar>> xei(input_data.num_emps,
                                  vector<BoolVar>(num_horizon_intervals));
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int i = 0; i < num_horizon_intervals; i++) {
            xei[e][i] = cp_model.NewBoolVar();
        }
    }


    //1_c_2
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            cp_model.AddLessOrEqual(zed[e][d], ted[e][d]);
            cp_model.AddLessOrEqual(ted[e][d], zed[e][d]*num_all_intervals);
        }
    }

    //1_e
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            cp_model.AddEquality(ned[e][d] + ted[e][d], ued[e][d]);
        }
    }

    //1_f
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 0; d < input_data.num_periods+1; d++) {
            cp_model.AddLessOrEqual(ned[e][d], zed[e][d] * 40);
            cp_model.AddGreaterOrEqual(ned[e][d], zed[e][d] * 24);
        }
    }


    //2
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = input_data.num_periods-1; d < input_data.num_periods+1; d++) {
            cp_model.AddLessOrEqual(ued[e][d], input_data.num_periods*96+24);
        }
    }


    //3
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 0; d < input_data.num_periods; d++) {
            cp_model.AddLessOrEqual(ued[e][d], ted[e][d+1] - 14*4 + (1-zed[e][d+1]) * num_all_intervals);
        }
    }

    //4
    for (int e = 0; e < input_data.num_emps; e++) {
        LinearExpr lhs = 0;

        for (int d = 0; d < input_data.num_periods+1; d++) {
            lhs += ned[e][d];
        }

        cp_model.AddLessOrEqual(std::get<0>(input_data.employee_workloads[e]), lhs);
        cp_model.AddGreaterOrEqual(std::get<1>(input_data.employee_workloads[e]), lhs);
    }

    //5
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 0; d < input_data.num_periods-4; d++) {
            LinearExpr lhs = 0;

            for (int j = d; j < d+6; j++)
            {
                lhs += zed[e][j];
            }

            cp_model.AddLessOrEqual(lhs, 5);
        }
    }


    //6
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int t = 0; t < input_data.num_tasks; t++) {
            int j = num_horizon_intervals - 1;
            cp_model.AddGreaterOrEqual(xeia[e][j-3][t] + xeia[e][j-2][t] + xeia[e][j-1][t], xeia[e][j][t] * 3);

            for (int i = num_horizon_intervals - 2; i >= 3; i--)
            {
                cp_model.AddGreaterOrEqual(xeia[e][i-3][t] + xeia[e][i-2][t] + xeia[e][i-1][t], 3 * (xeia[e][i][t] - xeia[e][i+1][t]));
            }
            cp_model.AddGreaterOrEqual(3 * xeia[e][3][t], xeia[e][2][t] + xeia[e][1][t] + xeia[e][0][t]);
            cp_model.AddGreaterOrEqual(2 * xeia[e][2][t], xeia[e][1][t] + xeia[e][0][t]);
            cp_model.AddGreaterOrEqual(xeia[e][1][t], xeia[e][0][t]);
        }
    }


    //7-a
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int i = 0; i < num_horizon_intervals; i++) {
            LinearExpr lhs = xei[e][i];

            for (int a = 0; a < input_data.num_tasks; a++)
            {
                lhs -= xeia[e][i][a];
            }

            cp_model.AddEquality(lhs, 0);
        }
    }

    //7-b
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int i = 0; i < 96-24; i++)
        {
            int offset_i = i + 24;
            BoolVar is_after_start = cp_model.NewBoolVar();
            cp_model.AddLessOrEqual(ted[e][0], offset_i + (1 - is_after_start) * num_all_intervals);
            cp_model.AddGreaterOrEqual(ted[e][0], offset_i + 1 - is_after_start * num_all_intervals);

            BoolVar is_before_end = cp_model.NewBoolVar();
            cp_model.AddGreaterOrEqual(ued[e][0], offset_i + 1 - (1 - is_before_end) * num_all_intervals);
            cp_model.AddLessOrEqual(ued[e][0], offset_i + is_before_end * num_all_intervals);

            cp_model.AddEquality(xei[e][i] + 1, is_after_start + is_before_end);
        }
    }

    //7-c
    for (int e = 0; e < input_data.num_emps; e++) {
        for (int d = 1; d < input_data.num_periods+1; d++) {
            int i_max = 96-24;
            if (d==input_data.num_periods)
                i_max = 0;

            for (int i = -24; i < i_max; i++) {
                int idx = d * 96 + i;
                int offset_i = idx + 24;

                BoolVar is_after_start = cp_model.NewBoolVar();
                cp_model.AddLessOrEqual(ted[e][d], offset_i + (1 - is_after_start) * num_all_intervals);
                cp_model.AddGreaterOrEqual(ted[e][d], offset_i + 1 - is_after_start * num_all_intervals);

                BoolVar is_before_end = cp_model.NewBoolVar();
                cp_model.AddGreaterOrEqual(ued[e][d], offset_i + 1 - (1 - is_before_end) * num_all_intervals);
                cp_model.AddLessOrEqual(ued[e][d], offset_i + is_before_end * num_all_intervals);

                BoolVar is_scheduled = cp_model.NewBoolVar();
                cp_model.AddEquality(is_scheduled + 1, is_after_start + is_before_end);


                BoolVar prev_is_after_start = cp_model.NewBoolVar();
                cp_model.AddLessOrEqual(ted[e][d-1], offset_i + (1 - prev_is_after_start) * num_all_intervals);
                cp_model.AddGreaterOrEqual(ted[e][d-1], offset_i + 1 - prev_is_after_start * num_all_intervals);

                BoolVar prev_is_before_end = cp_model.NewBoolVar();
                cp_model.AddGreaterOrEqual(ued[e][d-1], offset_i + 1 - (1 - prev_is_before_end) * num_all_intervals);
                cp_model.AddLessOrEqual(ued[e][d-1], offset_i + prev_is_before_end * num_all_intervals);

                BoolVar is_prev_scheduled = cp_model.NewBoolVar();
                cp_model.AddEquality(is_prev_scheduled + 1,  prev_is_after_start + prev_is_before_end);

                cp_model.AddEquality(is_prev_scheduled + is_scheduled, xei[e][idx]);
            }
        }
    }


    LinearExpr objective;
    //8
    for (int i = 0; i < num_horizon_intervals; i++) {
        for (int a = 0; a < input_data.num_tasks; a++)
        {
            LinearExpr cover_expr = 0;

            for (int e = 0; e < input_data.num_emps; e++)
            {
                cover_expr += xeia[e][i][a];
            }

            auto cover = cp_model.NewIntVar({std::get<0>(input_data.cover_requirements[i+24][a]), input_data.num_emps});
            cp_model.AddEquality(cover, cover_expr);

            auto upper_task_requirement = std::get<1>(input_data.cover_requirements[i+24][a]);
            for (int overstaff = upper_task_requirement + 1; overstaff <= input_data.num_emps; overstaff++)
            {
                auto is_over_cover = cp_model.NewBoolVar();

                cp_model.AddGreaterOrEqual(cover, overstaff).OnlyEnforceIf(is_over_cover);
                cp_model.AddLessOrEqual(cover, overstaff-1).OnlyEnforceIf(is_over_cover.Not());


                auto e_surplus = overstaff - upper_task_requirement;
                auto cost = (e_surplus * e_surplus) - ((e_surplus - 1) * (e_surplus - 1));
                objective += is_over_cover * cost;
            }
        }
    }
    cp_model.Minimize(objective);


    FILE* file = freopen(log_file_name.c_str(), "w", stdout);
    if (file == nullptr) {
        perror("Failed to redirect stdout");
        return;
    }


    Model model;
    SatParameters parameters;
    parameters.set_log_search_progress(true);
    parameters.set_random_seed(1);
    parameters.set_max_time_in_seconds(time_limit);
    model.Add(NewSatParameters(parameters));

    const CpSolverResponse response = SolveCpModel(cp_model.Build(), &model);


    for (int e = 0; e < input_data.num_emps; ++e)
    {
        for (int d = 0; d < input_data.num_periods+1; ++d)
        {
            auto& shift = outputData.shifts[d][e];

            shift.activity_count = 0;

            if (!SolutionBooleanValue(response, zed[e][d]))
                continue;

            shift.start = SolutionIntegerValue(response, ted[e][d]);


            int end_slot = SolutionIntegerValue(response, ued[e][d]);
            int last_task = 0;
            int task_length = 0;
            for (int time_slot = shift.start; time_slot < end_slot; time_slot++)
            {
                int slot_index = time_slot - 24;

                int task = -1;
                for (int t = 0; t < input_data.num_tasks; ++t)
                {
                    if (SolutionBooleanValue(response, xeia[e][slot_index][t])) {
                        task = t;
                        break;
                    }
                }


                if (task != last_task)
                {
                    if (task_length != 0)
                    {
                        ++shift.activity_count;
                        auto& act = shift.activities[shift.activity_count-1];
                        act.length = task_length;
                        act.task = last_task;
                    }
                    task_length = 1;
                }
                else
                    task_length += 1;

                last_task = task;
            }

            ++shift.activity_count;
            auto& act = shift.activities[shift.activity_count-1];
            act.length = task_length;
            act.task = last_task;
        }
    }

    fclose(file);

}


