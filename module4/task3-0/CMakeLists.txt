cmake_minimum_required(VERSION 3.22)
project(task3_0 C)

set(CMAKE_C_STANDARD 11)

find_package(OpenSSL COMPONENTS Crypto REQUIRED)

add_executable(task3_0
        main.c)
