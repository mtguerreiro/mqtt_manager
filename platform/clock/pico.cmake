cmake_minimum_required(VERSION 3.12)

if(PICO_BOARD STREQUAL "pico_w")
    add_subdirectory(picow)
else()
    add_subdirectory(pico)
endif()
