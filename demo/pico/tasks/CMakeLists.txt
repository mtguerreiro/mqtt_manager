cmake_minimum_required(VERSION 3.15)

add_library(
    tasks
    task_led.c
    task_blink.c
    task_temperature.c
    task_wiznet_init.c
    task_mqtt_mng.c
    )

target_link_libraries(
    tasks
    pico_stdlib hardware_spi
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    mdrivers
    mqtt_manager
)

target_include_directories(
    tasks
    PRIVATE 
    ${PROJECT_SOURCE_DIR} 
    ${MDRIVERS_PATH}
    ${MQTT_MNG_PATH}/clogging
)
