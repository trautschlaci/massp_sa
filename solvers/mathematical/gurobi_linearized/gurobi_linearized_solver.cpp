#include "gurobi_linearized_solver.h"
#include "gurobi_c++.h"
#include "../utils.h"


void GurobiLinearizedSolver::solve_problem(OutputData& outputData, const std::string& log_file_name, double time_limit) {
    try {
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", log_file_name);
        env.start();

        GRBModel model = GRBModel(env);


        int num_horizon_intervals = input_data.num_periods*96;
        int num_all_intervals = num_horizon_intervals + 96;


        vector<vector<vector<GRBVar>>> xeia(input_data.num_emps,
                                              vector<vector<GRBVar>>(num_horizon_intervals,
                                                      vector<GRBVar>(input_data.num_tasks)));
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int i = 0; i < num_horizon_intervals; i++) {
                for (int t = 0; t < input_data.num_tasks; t++) {
                    xeia[e][i][t] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
                }
            }
        }

        vector<vector<GRBVar>> ted(input_data.num_emps,
                                              vector<GRBVar>(input_data.num_periods+1));
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                int base = d * 96;

                ted[e][d] = model.addVar(0, base + 23 * 4 + 3, 0, GRB_INTEGER, "");

                if (d > 0)
                {
                    auto is_after_midnight = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
                    model.addConstr(base >= ted[e][d] - is_after_midnight * num_all_intervals);
                    model.addConstr(base + 6 * 4 <= ted[e][d] + (1 - is_after_midnight) * num_all_intervals);
                }

                auto is_after_ten = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
                model.addConstr(base + 10 * 4 >= ted[e][d] - is_after_ten * num_all_intervals);
                model.addConstr(base + 14 * 4 <= ted[e][d] + (1 - is_after_ten) * num_all_intervals);

                auto is_after_eighteen = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
                model.addConstr(base + 18 * 4 >= ted[e][d] - is_after_eighteen * num_all_intervals);
                model.addConstr(base + 20 * 4 <= ted[e][d] + (1 - is_after_eighteen) * num_all_intervals);
            }
        }

        vector<vector<GRBVar>> ned(input_data.num_emps,
                                              vector<GRBVar>(input_data.num_periods+1));
        for (int t = 0; t < input_data.num_emps; t++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                ned[t][d] = model.addVar(0, 40, 0, GRB_INTEGER, "");
            }
        }

        vector<vector<GRBVar>> ued(input_data.num_emps,
                                      vector<GRBVar>(input_data.num_periods+1));
        for (int t = 0; t < input_data.num_emps; t++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                ued[t][d] = model.addVar(0, num_all_intervals, 0, GRB_INTEGER, "");
            }
        }

        vector<vector<GRBVar>> zed(input_data.num_emps,
                              vector<GRBVar>(input_data.num_periods+1));
        for (int t = 0; t < input_data.num_emps; t++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                zed[t][d] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
            }
        }

        vector<vector<GRBVar>> xei(input_data.num_emps,
                                      vector<GRBVar>(num_horizon_intervals));
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int i = 0; i < num_horizon_intervals; i++) {
                xei[e][i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "");
            }
        }


        //1_c_2
        for (int e = 0; e < input_data.num_emps; e++) {
            model.addConstr(zed[e][0] * 6 * 4 <= ted[e][0], "1-c-2");
            model.addConstr(ted[e][0] <= zed[e][0] * num_all_intervals, "1-c-2-b");
            for (int d = 1; d < input_data.num_periods+1; d++) {
                model.addConstr(zed[e][d] * d * 96 <= ted[e][d], "1-c-2");
                model.addConstr(ted[e][d] <= zed[e][d] * num_all_intervals, "1-c-2-b");
            }
        }


        //1_e
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                model.addConstr(ned[e][d] + ted[e][d] == ued[e][d], "1-e");
            }
        }

        //1_f
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                model.addConstr(ned[e][d] <= zed[e][d] * 40, "1-f-1");
                model.addConstr(ned[e][d] >= zed[e][d] * 24, "1-f-2");
            }
        }


        //2
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = input_data.num_periods-1; d < input_data.num_periods+1; d++) {
                model.addConstr(ued[e][d] <= input_data.num_periods*96+24, "2");
            }
        }


        //3
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods; d++) {
                model.addConstr(ued[e][d] <= ted[e][d+1] - 14*4 + (1-zed[e][d+1]) * num_all_intervals, "3");
            }
        }

        //4
        for (int e = 0; e < input_data.num_emps; e++) {
            GRBLinExpr lhs = 0;

            for (int d = 0; d < input_data.num_periods+1; d++) {
                lhs += ned[e][d];
            }

            model.addConstr(std::get<0>(input_data.employee_workloads[e]) <= lhs, "4-1");
            model.addConstr(std::get<1>(input_data.employee_workloads[e]) >= lhs, "4-2");
        }

        //5
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods-4; d++) {
                GRBLinExpr lhs = 0;

                for (int j = d; j < d+6; j++)
                {
                    lhs += zed[e][j];
                }

                model.addConstr(lhs <= 5, "5");
            }
        }

        //6
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int t = 0; t < input_data.num_tasks; t++) {
                int j = num_horizon_intervals - 1;
                model.addConstr(xeia[e][j-3][t] + xeia[e][j-2][t] + xeia[e][j-1][t] >= xeia[e][j][t] * 3);

                for (int i = num_horizon_intervals - 2; i >= 3; i--)
                {
                    model.addConstr(xeia[e][i-3][t] + xeia[e][i-2][t] + xeia[e][i-1][t] >= 3 * (xeia[e][i][t] - xeia[e][i+1][t]));
                }
                model.addConstr(3 * xeia[e][3][t] >= xeia[e][2][t] + xeia[e][1][t] + xeia[e][0][t]);
                model.addConstr(2 * xeia[e][2][t] >= xeia[e][1][t] + xeia[e][0][t]);
                model.addConstr(xeia[e][1][t] >= xeia[e][0][t]);
            }
        }

        //7-a
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int i = 0; i < num_horizon_intervals; i++) {
                GRBLinExpr lhs = xei[e][i];

                for (int a = 0; a < input_data.num_tasks; a++)
                {
                    lhs -= xeia[e][i][a];
                }

                model.addConstr(lhs == 0, "7-a");
            }
        }

        //7-b
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int i = 0; i < 96-24; i++)
            {
                int offset_i = i + 24;
                GRBVar is_after_start = model.addVar(0, 1, 0, GRB_BINARY, "Is_After_Start");
                model.addConstr(ted[e][0] <= offset_i + (1 - is_after_start) * num_all_intervals);
                model.addConstr(ted[e][0] >= offset_i + 1 - is_after_start * num_all_intervals);

                GRBVar is_before_end = model.addVar(0, 1, 0, GRB_BINARY, "Is_Before_End");
                model.addConstr(ued[e][0] >= offset_i + 1 - (1 - is_before_end) * num_all_intervals);
                model.addConstr(ued[e][0] <= offset_i + is_before_end * num_all_intervals);

                model.addConstr(xei[e][i] + 1 == is_after_start + is_before_end, "7-b");
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

                    GRBVar is_after_start = model.addVar(0, 1, 0, GRB_BINARY, "Is_After_Start");
                    model.addConstr(ted[e][d] <= offset_i + (1 - is_after_start) * num_all_intervals);
                    model.addConstr(ted[e][d] >= offset_i + 1 - is_after_start * num_all_intervals);

                    GRBVar is_before_end = model.addVar(0, 1, 0, GRB_BINARY, "Is_Before_End");
                    model.addConstr(ued[e][d] >= offset_i + 1 - (1 - is_before_end) * num_all_intervals);
                    model.addConstr(ued[e][d] <= offset_i + is_before_end * num_all_intervals);

                    GRBVar is_scheduled = model.addVar(0, 1, 0, GRB_BINARY, "Is_Scheduled");
                    model.addConstr(is_scheduled + 1 == is_after_start + is_before_end, "7-c-1");


                    GRBVar prev_is_after_start = model.addVar(0, 1, 0, GRB_BINARY, "Prev_Is_After_Start");
                    model.addConstr(ted[e][d-1] <= offset_i + (1 - prev_is_after_start) * num_all_intervals);
                    model.addConstr(ted[e][d-1] >= offset_i + 1 - prev_is_after_start * num_all_intervals);

                    GRBVar prev_is_before_end = model.addVar(0, 1, 0, GRB_BINARY, "Prev_Is_Before_End");
                    model.addConstr(ued[e][d-1] >= offset_i + 1 - (1 - prev_is_before_end) * num_all_intervals);
                    model.addConstr(ued[e][d-1] <= offset_i + prev_is_before_end * num_all_intervals);

                    GRBVar is_prev_scheduled = model.addVar(0, 1, 0, GRB_BINARY, "Is_Prev_Scheduled");
                    model.addConstr(is_prev_scheduled + 1 == prev_is_after_start + prev_is_before_end, "7-c-2");

                    model.addConstr(is_prev_scheduled + is_scheduled == xei[e][idx], "7-c-3");
                }
            }
        }


        GRBLinExpr objective;
        //8
        for (int i = 0; i < num_horizon_intervals; i++) {
            for (int a = 0; a < input_data.num_tasks; a++)
            {
                GRBLinExpr cover_expr = 0;

                for (int e = 0; e < input_data.num_emps; e++)
                {
                    cover_expr += xeia[e][i][a];
                }

                auto cover = model.addVar(std::get<0>(input_data.cover_requirements[i+24][a]), input_data.num_emps, 0, GRB_INTEGER, "Cover");
                model.addConstr(cover == cover_expr);

                auto upper_task_requirement = std::get<1>(input_data.cover_requirements[i+24][a]);
                for (int overstaff = upper_task_requirement + 1; overstaff <= input_data.num_emps; overstaff++)
                {
                    auto is_over_cover = model.addVar(0, 1, 0, GRB_BINARY, "Is_Over_Cover");

                    model.addConstr(cover >= overstaff - (1-is_over_cover)*input_data.num_emps);
                    model.addConstr(cover <= overstaff-1 + is_over_cover*input_data.num_emps);


                    auto e_surplus = overstaff - upper_task_requirement;
                    auto cost = (e_surplus * e_surplus) - ((e_surplus - 1) * (e_surplus - 1));
                    objective += is_over_cover * cost;
                }
            }
        }
        model.setObjective(objective, GRB_MINIMIZE);


        model.set(GRB_IntParam_Seed, 1);
        model.set(GRB_DoubleParam_TimeLimit, time_limit);

        model.optimize();


        for (int e = 0; e < input_data.num_emps; ++e)
        {
            for (int d = 0; d < input_data.num_periods+1; ++d)
            {
                auto& shift = outputData.shifts[d][e];

                shift.activity_count = 0;

                if (!(static_cast<bool>(zed[e][d].get(GRB_DoubleAttr_X))))
                    continue;

                shift.start = static_cast<int>(ted[e][d].get(GRB_DoubleAttr_X));


                int end_slot = static_cast<int>(ued[e][d].get(GRB_DoubleAttr_X));
                int last_task = 0;
                int task_length = 0;
                for (int time_slot = shift.start; time_slot < end_slot; time_slot++)
                {
                    int slot_index = time_slot - 24;

                    int task = -1;
                    for (int t = 0; t < input_data.num_tasks; ++t)
                    {
                        if (static_cast<bool>(xeia[e][slot_index][t].get(GRB_DoubleAttr_X)))
                        {
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




    } catch(GRBException& e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}

