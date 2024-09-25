add_library(branch_lib branch_bound.cpp)
add_library(knapsack_lib knapsacks.cpp)

target_sources(branch_lib PUBLIC branch_bound.hpp)
target_sources(knapsack_lib PUBLIC knapsacks.cpp)
