
add_executable(test-highs test.cpp test_main.cpp)
target_link_libraries(test-highs  knapsack_lib knapsack_highs_lib)
target_include_directories(test-highs
                            PUBLIC "${PROJECT_SOURCE_DIR}/my_libs"
                            PUBLIC  "${PROJECT_SOURCE_DIR}/test-bench")

enable_testing()
add_test(NAME test_highs
        COMMAND $<TARGET_FILE:test-highs>)
