cmake_minimum_required(VERSION 3.29)
project(ex1 C)

set(CMAKE_C_STANDARD 11)

add_executable(ex1 main.c
        sensor_db.c)
