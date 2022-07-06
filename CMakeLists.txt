cmake_minimum_required(VERSION 3.21)
project(ConwayLifeGame)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

find_package(OpenMP)

add_executable(ConwayLifeGame main.cpp)

if (OpenMP_FOUND)
    message("OpenMP")
    target_link_libraries(ConwayLifeGame PUBLIC OpenMP::OpenMP_CXX)
endif ()
