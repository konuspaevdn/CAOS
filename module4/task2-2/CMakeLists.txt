cmake_minimum_required(VERSION 3.22)
project(task2_2 C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)

pkg_check_modules(CURL REQUIRED libcurl)

include_directories(${CURL_INCLUDE_DIRECTORIES})
add_executable(task2_2
        main.c)
target_link_libraries(task2_2 ${CURL_LIBRARIES})
