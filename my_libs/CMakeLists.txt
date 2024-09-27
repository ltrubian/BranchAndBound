add_library(branch_lib branch_bound.cpp)
add_library(knapsack_lib knapsacks.cpp)
add_library(knapsack_highs_lib knapsacks_highs.cpp)

target_sources(branch_lib PUBLIC branch_bound.hpp)
target_sources(knapsack_lib PUBLIC knapsacks.hpp)
target_sources(knapsack_highs_lib PUBLIC knapsacks_highs.hpp)

set(HIGHS_DIR /usr/local/lib/cmake/highs)
find_package(HIGHS REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(knapsack_highs_lib  highs::highs)
