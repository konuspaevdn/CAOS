cmake_minimum_required(VERSION 3.22)
project(task5-0)
find_package(Python3 COMPONENTS Development REQUIRED)

set(CMAKE_C_STANDARD 11)

include_directories(${Python3_INCLUDE_DIRS})
add_executable(task5-0 main.c module.c)
target_link_libraries(task5-0 ${Python3_LIBRARIES})

python3_add_library(matrix MODULE module.c)