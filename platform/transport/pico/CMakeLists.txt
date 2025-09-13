cmake_minimum_required(VERSION 3.12)

add_library(plaintext_transport plaintext_transport.c)

target_include_directories(
    plaintext_transport 
    PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "../../../clogging"
    "../"
    ${MDRIVERS_PATH}
)

target_link_libraries(
    plaintext_transport
    PRIVATE
    pico_stdlib hardware_spi
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    mdrivers
)

