cmake_minimum_required(VERSION 3.22)
project(python_in_c)
find_package(Python3 COMPONENTS Development REQUIRED)

set(CMAKE_C_STANDARD 11)

include_directories(${Python3_INCLUDE_DIRS})
add_executable(python_in_c main.c)
target_link_libraries(python_in_c ${Python3_LIBRARIES})