cmake_minimum_required(VERSION 3.12)

add_library(plaintext_transport plaintext_transport.c sockets_impl.c)

target_include_directories(
    plaintext_transport PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "../../../logging"
    "../"
)

target_link_libraries(plaintext_transport PRIVATE
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
)
