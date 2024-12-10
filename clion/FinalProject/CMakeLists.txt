cmake_minimum_required(VERSION 3.30)
project(FinalProject C)

set(CMAKE_C_STANDARD 11)

add_executable(FinalProject main.c
        connmgr.c
        connmgr.h)
