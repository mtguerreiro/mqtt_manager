cmake_minimum_required(VERSION 3.12)

add_library(tasks
    task_blink.c
    task_temperature.c
    task_wiznet_init.c
    task_svmqtt.c
    )

target_link_libraries(tasks
    pico_stdlib hardware_spi
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    tif
    temperature led
    hw_pico_modules
    mqtt_manager)

target_include_directories(
    tasks
    PRIVATE 
    "${PROJECT_SOURCE_DIR}" 
    "${TIF_PATH}"
    ${MQTT_MNG_PATH}/logging
)
