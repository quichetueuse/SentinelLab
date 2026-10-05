cmake_minimum_required(VERSION 3.15)
project(noeud CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(noeud main.cpp SimSensor.h)

if(MSVC)
    target_compile_options(noeud PRIVATE /W4)
else()
    target_compile_options(noeud PRIVATE -Wall -Wextra)
endif()