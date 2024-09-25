cmake_minimum_required(VERSION 3.15)
project(Branch_and_Bound)
set(CMAKE_CXX_FLAGS "-O3")

add_subdirectory(my_libs)

add_executable(main main.cpp)

target_link_libraries(main PUBLIC branch_lib)
target_link_libraries(main PUBLIC knapsack_lib)
target_include_directories(main PUBLIC
		"${PROJECT_SOURCE_DIR}/my_libs")
		
		
file(GLOB_RECURSE ALL_SOURCE_FILES
    *.c *.h *.cpp *.hpp *.cxx *.hxx *.cc *.hh *.cppm *.ipp *.ixx)
add_custom_target(format
    COMMAND clang-format
    -i
    ${ALL_SOURCE_FILES}
)
