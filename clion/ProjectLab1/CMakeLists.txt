cmake_minimum_required(VERSION 3.29)
project(ProjectLab1 C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-DDEBUG)

add_executable(ProjectLab1 main.c
        lib/dplist.c
        file_creator.c
        datamgr.c)
