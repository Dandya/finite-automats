cmake_minimum_required(VERSION 3.0)

project(automats_labs)

set(CMAKE_VERBOSE_MAKEFILE ON)

# add_subdirectory(contrib/matrix EXCLUDE_FROM_ALL)

add_subdirectory(contrib/eigen Eigen3)

add_compile_options(-std=c++20 -Wall)

set(SOURCES src/main.cpp src/rs_atmt.cpp src/lin_atmt.cpp)

add_executable(automats ${SOURCES})

# target_include_directories(automats PRIVATE contrib/matrix/include)
target_link_libraries(automats PRIVATE Eigen3)