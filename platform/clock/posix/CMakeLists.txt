cmake_minimum_required(VERSION 3.12)

add_library(clock clock_posix.c)

target_include_directories(
    clock PRIVATE
    "../"
)
