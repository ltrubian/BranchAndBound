



add_subdirectory(highs)


add_executable(benchmark benchmark.cpp)

target_link_libraries(benchmark PUBLIC knapsack_lib)
target_include_directories(benchmark PUBLIC
		"${PROJECT_SOURCE_DIR}/my_libs")
