cmake_minimum_required(VERSION 3.12)

add_library(
    threads
    task_blink.c
    task_mqtt_mng.c
    task_temperature.c
    task_led.c
)

target_link_libraries(
    threads
    PRIVATE
    mqtt_manager
    #rt
    #pigpio
    mdrivers
)

target_include_directories(
    threads
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${MQTT_MNG_PATH}/clogging
    ${MDRIVERS_PATH}
)
