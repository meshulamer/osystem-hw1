cmake_minimum_required(VERSION 3.15)
project(hw1)

set(CMAKE_CXX_STANDARD 14)

add_executable(hw1 smash.cpp Commands.h Commands.cpp signals.cpp signals.h)