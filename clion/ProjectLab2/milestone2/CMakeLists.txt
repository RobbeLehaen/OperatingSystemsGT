cmake_minimum_required(VERSION 3.29)
project(milestone2 C)

set(CMAKE_C_STANDARD 11)

add_executable(milestone2 main.c
        logger.c
        sensor_db.c)
