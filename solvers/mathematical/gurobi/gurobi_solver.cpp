#include "gurobi_solver.h"
#include "gurobi_c++.h"
#include "../utils.h"


void GurobiSolver::solve_problem(OutputData& outputData, const std::string& log_file_name, double time_limit) {
    try {
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", log_file_name);
        env.start();

        GRBModel model = GRBModel(env);


        int num_horizon_intervals = input_data.num_periods*96;
        int num_all_intervals = num_horizon_intervals + 96;


        vector<vector<int>> sd(input_data.num_periods+1, vector<int>(0));
        auto vec_1 = create_range(6*4, 10*4);
        auto vec_2 = create_range(14*4, 18*4);
        auto vec_3 = create_range(20*4, 23*4+3);
        sd[0] = vec_1;
        sd[0].insert(sd[0].end(), vec_2.begin(), vec_2.end());
        sd[0].insert(sd[0].end(), vec_3.begin(), vec_3.end());
        for (int d = 1; d < input_data.num_periods; d++) {
            int base = d * 96;
            sd[d] = {base};
            vec_1 = create_range(base + 6 * 4, base + 10 * 4),
            vec_2 = create_range(base + 14 * 4, base + 18 * 4),
            vec_3 = create_range(base + 20 * 4, base + 23 * 4 + 3);
            sd[d].insert(sd[d].end(), vec_1.begin(), vec_1.end());
            sd[d].insert(sd[d].end(), vec_2.begin(), vec_2.end());
            sd[d].insert(sd[d].end(), vec_3.begin(), vec_3.end());
        }
        sd[input_data.num_periods] = {input_data.num_periods*96};


        vector<int> L_set = {0};
        vec_1 = create_range(24, 40);
        L_set.insert(L_set.end(), vec_1.begin(), vec_1.end());


        vector<vector<vector<GRBVar>>> peds(input_data.num_emps,
                                      vector<vector<GRBVar>>(input_data.num_periods+1,
                                              vector<GRBVar>(0)));
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                for (int s = 0; s < sd[d].size(); s++) {
                    peds[e][d].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ""));
                }
            }
        }

        vector<vector<vector<GRBVar>>> qedl(input_data.num_emps,
                              vector<vector<GRBVar>>(input_data.num_periods+1,
                                      vector<GRBVar>(0)));
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                for (int l = 0; l < L_set.size(); l++) {
                    qedl[e][d].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, ""));
                }
            }
        }

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

        vector<vector<GRBVar>> yia(num_horizon_intervals,
                              vector<GRBVar>(input_data.num_tasks));
        for (int i = 0; i < num_horizon_intervals; i++) {
            for (int t = 0; t < input_data.num_tasks; t++) {
                yia[i][t] = model.addVar(0, input_data.num_emps, 0, GRB_INTEGER, "");
            }
        }



        vector<vector<GRBVar>> ted(input_data.num_emps,
                                              vector<GRBVar>(input_data.num_periods+1));
        for (int t = 0; t < input_data.num_emps; t++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                ted[t][d] = model.addVar(0, num_all_intervals, 0, GRB_INTEGER, "");
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



        //1_a
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                GRBLinExpr lhs = 0;

                for (int s = 0; s < sd[d].size(); s++) {
                    lhs += peds[e][d][s];
                }

                model.addConstr(lhs <= 1, "1-a");
            }
        }

        //1_b
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                GRBLinExpr lhs = 0;

                for (int l = 0; l < L_set.size(); l++) {
                    lhs += qedl[e][d][l];
                }

                model.addConstr(lhs <= 1, "1-b");
            }
        }

        //1_c
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                GRBLinExpr lhs = 0;

                for (int s = 0; s < sd[d].size(); s++) {
                    lhs += peds[e][d][s] * sd[d][s];
                }

                model.addConstr(lhs == ted[e][d], "1-c");
            }
        }

        //1_c_2
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                model.addConstr(zed[e][d] <= ted[e][d], "1-c-2");
                model.addConstr(ted[e][d] <= zed[e][d]*num_all_intervals, "1-c-2-b");
            }
        }

        //1_d
        for (int e = 0; e < input_data.num_emps; e++) {
            for (int d = 0; d < input_data.num_periods+1; d++) {
                GRBLinExpr lhs = 0;

                for (int l = 0; l < L_set.size(); l++) {
                    lhs += qedl[e][d][l] * L_set[l];
                }

                model.addConstr(lhs == ned[e][d], "1-d");
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


        //8
        for (int i = 0; i < num_horizon_intervals; i++) {
            for (int a = 0; a < input_data.num_tasks; a++)
            {
                GRBLinExpr lhs = -yia[i][a];

                for (int e = 0; e < input_data.num_emps; e++)
                {
                    lhs += xeia[e][i][a];
                }

                model.addConstr(lhs >= std::get<0>(input_data.cover_requirements[i+24][a]), "8-1");
                model.addConstr(lhs <= std::get<1>(input_data.cover_requirements[i+24][a]), "8-2");
            }
        }



        GRBQuadExpr objective;
        for (int i = 0; i < num_horizon_intervals; i++) {
            for (int a = 0; a < input_data.num_tasks; a++) {
                objective += yia[i][a] * yia[i][a];
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

