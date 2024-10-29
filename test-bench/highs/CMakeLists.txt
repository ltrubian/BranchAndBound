

add_library(test_main test_main.cpp)

target_include_directories(test_main
                            PUBLIC  "${PROJECT_SOURCE_DIR}/test-bench")


add_executable(test-relax test_relaxed.cpp )
target_link_libraries(test-relax  knapsack_lib knapsack_highs_lib test_main)
target_include_directories(test-relax
                            PUBLIC "${PROJECT_SOURCE_DIR}/my_libs"
                            PUBLIC  "${PROJECT_SOURCE_DIR}/test-bench")



add_executable(test-integer test_integer.cpp )
target_link_libraries(test-integer  knapsack_lib knapsack_highs_lib test_main)
target_include_directories(test-integer
                            PUBLIC "${PROJECT_SOURCE_DIR}/my_libs"
                            PUBLIC  "${PROJECT_SOURCE_DIR}/test-bench")


add_executable(test-mixed-integer test_mixed_integer.cpp )
target_link_libraries(test-mixed-integer  knapsack_lib knapsack_highs_lib test_main)
target_include_directories(test-mixed-integer
                            PUBLIC "${PROJECT_SOURCE_DIR}/my_libs"
                            PUBLIC  "${PROJECT_SOURCE_DIR}/test-bench")
