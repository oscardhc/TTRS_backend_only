cmake_minimum_required(VERSION 3.10)
project(ttrs)

add_compile_options(-Ofast)
set(CMAKE_CXX_STANDARD 17)

add_executable(code fly/test.cpp)
