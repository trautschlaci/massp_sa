cmake_minimum_required(VERSION 3.20)
project(multi_activity_sa_cpp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")

set(CMAKE_CXX_STANDARD 20)

set(GUROBI_HOME "C:/gurobi1200/win64")
include_directories(${GUROBI_HOME}/include)

add_library(CPLEX::CPLEX UNKNOWN IMPORTED)
set_target_properties(CPLEX::CPLEX PROPERTIES
        IMPORTED_LOCATION "C:/Program Files/IBM/ILOG/CPLEX_Studio2211/cplex/lib/x64_windows_msvc14/stat_mda/cplex2211.lib"
        INTERFACE_INCLUDE_DIRECTORIES "C:/Program Files/IBM/ILOG/CPLEX_Studio2211/cplex/include"
)

set(ortools_DIR "C:/Program Files (x86)/ortools/")
set(ZLIB_INCLUDE_DIR "C:/Program Files (x86)/ortools/include")
set(ZLIB_LIBRARY "C:/Program Files (x86)/ortools/lib/zlib.lib")
set(ABSL_INCLUDE_DIR "C:/Program Files (x86)/ortools/include")
set(Protobuf_DIR "C:/Program Files (x86)/ortools/cmake")
set(CMAKE_PREFIX_PATH "C:/Program Files (x86)/ortools/lib/cmake")
include_directories(${CPLEX_ROOT}/include)

find_package(ortools CONFIG REQUIRED)

include_directories(libs)
include_directories(IO)
include_directories(solvers/simulated_annealing)

link_directories(${GUROBI_HOME}/lib)

add_executable(multi_activity_sa_cpp main.cpp
        utils.h
        IO/problem_constants.h
        IO/problem_constants.cpp
        IO/input_data.h
        IO/input_data.cpp
        IO/output_data.cpp
        IO/output_data.h
        solvers/mathematical/gurobi/gurobi_solver.cpp
        solvers/mathematical/gurobi/gurobi_solver.h
        solvers/mathematical/cp_sat/cp_sat_solver.cpp
        solvers/mathematical/cp_sat/cp_sat_solver.h
        solvers/mathematical/cp_sat_linearized/cp_sat_linearized_solver.h
        solvers/mathematical/cp_sat_linearized/cp_sat_linearized_solver.cpp
        solvers/mathematical/gurobi_linearized/gurobi_linearized_solver.h
        solvers/mathematical/gurobi_linearized/gurobi_linearized_solver.cpp
        solvers/simulated_annealing/heuristics/fast_sa_algorithm.h
        solvers/simulated_annealing/heuristics/fast_sa_algorithm.cpp
        solvers/simulated_annealing/manager/state_manager.cpp
        solvers/simulated_annealing/manager/state_manager.h
        solvers/simulated_annealing/moves/move_base.h
        solvers/simulated_annealing/model/state.cpp
        solvers/simulated_annealing/model/state.h
        solvers/simulated_annealing/hyper_parameters.h
        solvers/simulated_annealing/moves/move_enum.h
        solvers/simulated_annealing/sa_solver.cpp
        solvers/simulated_annealing/sa_solver.h
        solvers/simulated_annealing/moves/move_types/shift_add_move.cpp
        solvers/simulated_annealing/moves/move_types/shift_add_move.h
        solvers/simulated_annealing/moves/move_helpers/random_selector.cpp
        solvers/simulated_annealing/moves/move_helpers/random_selector.h
        solvers/simulated_annealing/moves/move_helpers/cost_inspector.cpp
        solvers/simulated_annealing/moves/move_helpers/cost_inspector.h
        solvers/simulated_annealing/moves/move_types/shift_remove_move.cpp
        solvers/simulated_annealing/moves/move_types/shift_remove_move.h
        solvers/simulated_annealing/moves/move_types/shift_start_change_move.cpp
        solvers/simulated_annealing/moves/move_types/shift_start_change_move.h
        solvers/simulated_annealing/moves/move_types/shift_swap_move.cpp
        solvers/simulated_annealing/moves/move_types/shift_swap_move.h
        solvers/simulated_annealing/moves/move_types/activity_length_change_move.cpp
        solvers/simulated_annealing/moves/move_types/activity_length_change_move.h
        solvers/simulated_annealing/moves/move_types/activity_task_change_move.cpp
        solvers/simulated_annealing/moves/move_types/activity_task_change_move.h
        solvers/simulated_annealing/moves/move_types/activity_add_move.cpp
        solvers/simulated_annealing/moves/move_types/activity_add_move.h
        solvers/simulated_annealing/moves/move_types/activity_remove_move.cpp
        solvers/simulated_annealing/moves/move_types/activity_remove_move.h
        solvers/simulated_annealing/moves/move_types/shift_swap_period_move.cpp
        solvers/simulated_annealing/moves/move_types/shift_swap_period_move.h
        solvers/simulated_annealing/heuristics/cutoff_sa_algorithm.cpp
        solvers/simulated_annealing/heuristics/cutoff_sa_algorithm.h
        solvers/simulated_annealing/heuristics/sa_base.h
)
add_library(multi_activity_sa_cpp_lib libs/pugixml.hpp libs/pugiconfig.hpp libs/pugixml.cpp)

target_link_libraries(multi_activity_sa_cpp gurobi_c++md2017 gurobi120)

target_link_libraries(multi_activity_sa_cpp ortools::ortools)

target_link_libraries(multi_activity_sa_cpp ${CPLEX_LIBRARY})
