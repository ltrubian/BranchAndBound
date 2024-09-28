cmake_minimum_required(VERSION 3.15)
project(Branch_and_Bound)
set(CMAKE_CXX_FLAGS "-O3 -std=c++11")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(my_libs)

add_executable(main main.cpp)

target_link_libraries(main PUBLIC knapsack_lib)
target_link_libraries(main PUBLIC knapsack_highs_lib)
target_include_directories(main PUBLIC
		"${PROJECT_SOURCE_DIR}/my_libs")


set(HIGHS_DIR /usr/local/lib/cmake/highs)
find_package(HIGHS REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(knapsack_highs_lib  highs::highs)

		
file(GLOB_RECURSE ALL_SOURCE_FILES
    *.c *.h *.cpp *.hpp *.cxx *.hxx *.cc *.hh *.cppm *.ipp *.ixx)
add_custom_target(format
    COMMAND clang-format
    -i
    ${ALL_SOURCE_FILES}
)
add_custom_target(cmake-refresh
    COMMAND rm  CMakeCache.txt
    COMMAND rm  compile_commands.json
    COMMAND cmake .
)
