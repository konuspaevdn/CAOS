cmake_minimum_required(VERSION 3.22)
project(task4-1 C)
find_package(PkgConfig REQUIRED)

pkg_check_modules(FUSE REQUIRED fuse3)

include_directories(${FUSE_INCLUDE_DIRS})

set(CMAKE_C_STANDARD 11)

add_executable(prog main.c)
target_link_libraries(prog ${FUSE_LIBRARIES})
