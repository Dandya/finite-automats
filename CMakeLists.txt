cmake_minimum_required(VERSION 3.0)

project(automats_labs)

add_compile_options(-std=c++20 -Wall)

set(SOURCES src/main.cpp src/rs_atmt.cpp)

add_executable(automats ${SOURCES})