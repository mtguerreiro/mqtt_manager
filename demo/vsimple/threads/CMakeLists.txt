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
        subscription_manager
        clock
        plaintext_transport
)
target_include_directories(
    threads
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${MQTT_INCLUDE_PUBLIC_DIRS}
        ${SVMQTT_PATH}/platform/transport
        ${SVMQTT_PATH}/platform/clock
        ${SVMQTT_PATH}/logging
        ${SVMQTT_PATH}/subscription_manager
)