cmake_minimum_required(VERSION 3.22)
project(task5-1)
find_package(Python3 COMPONENTS Development REQUIRED)

set(CMAKE_C_STANDARD 11)

include_directories(${Python3_INCLUDE_DIRS})
add_executable(task5-1 main.c)
target_link_libraries(task5-1 ${Python3_LIBRARIES})

python3_add_library(primes MODULE module.c)
