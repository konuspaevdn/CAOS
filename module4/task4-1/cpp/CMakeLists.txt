cmake_minimum_required(VERSION 3.22)
project(task4-1)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FUSE REQUIRED fuse3)

include_directories(${FUSE_INCLUDE_DIRS})

add_executable(prog main.cpp)
target_link_libraries(prog ${FUSE_LIBRARIES})
