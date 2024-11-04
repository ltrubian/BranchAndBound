
add_library(utilities_lib INTERFACE)

target_sources(utilities_lib INTERFACE utilities.hpp)


add_library(interface INTERFACE)

target_sources(interface INTERFACE interface.hpp)


add_library(knapsack_lib knapsacks.cpp)

target_sources(knapsack_lib PUBLIC knapsacks.hpp)

add_library(knapsack_highs_lib knapsacks_highs.cpp)

target_sources(knapsack_highs_lib PUBLIC knapsacks_highs.hpp)




target_link_libraries(knapsack_highs_lib utilities_lib)
target_link_libraries(knapsack_lib utilities_lib)

