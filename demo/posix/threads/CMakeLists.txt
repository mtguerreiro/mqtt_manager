cmake_minimum_required(VERSION 3.12)

add_library(
    threads
    task_blink.c
    task_svmqtt.c
    task_temperature.c
    task_led.c
)

target_link_libraries(
    threads
    PRIVATE
    mqtt_manager
)

target_include_directories(
    threads
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${MQTT_MNG_PATH}/logging
)
