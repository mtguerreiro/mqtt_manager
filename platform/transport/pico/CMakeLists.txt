cmake_minimum_required(VERSION 3.12)

add_library(plaintext_transport plaintext_transport.c)

target_include_directories(
    plaintext_transport PUBLIC
    "${PROJECT_SOURCE_DIR}"
    "../../../logging"
    "../"
    "${TIF_PATH}"
)

target_link_libraries(plaintext_transport PRIVATE
    pico_stdlib hardware_spi
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    tif
    hw_pico_modules
)

