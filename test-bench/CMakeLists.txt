



add_subdirectory(highs)

add_library(argparse INTERFACE)
target_sources(argparse INTERFACE argparse.hpp)

add_executable(benchmark benchmark.cpp)

target_link_libraries(benchmark PUBLIC knapsack_lib )#argparse)
target_include_directories(benchmark PUBLIC
		"${PROJECT_SOURCE_DIR}/my_libs")

add_executable(one-run one-run.cpp)

target_link_libraries(one-run PUBLIC knapsack_lib argparse)
target_include_directories(one-run PUBLIC
		"${PROJECT_SOURCE_DIR}/my_libs")
