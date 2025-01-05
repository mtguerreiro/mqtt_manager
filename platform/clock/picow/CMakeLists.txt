cmake_minimum_required(VERSION 3.12)

add_library(clock clock_picow.c)

target_include_directories(
    clock PRIVATE
    "../"
)

target_link_libraries(clock PRIVATE
    pico_stdlib
)


