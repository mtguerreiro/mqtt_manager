cmake_minimum_required(VERSION 3.12)

add_library(${DEMO_NAME_TASKS}
    task_blink.c
    task_wifi_init.c
    task_svmqtt.c
    task_temperature.c
    task_led.c
    )

target_compile_definitions(${DEMO_NAME_TASKS} PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
    )

target_link_libraries(${DEMO_NAME_TASKS}
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    tif 
    temperature led
    modbus
    hw_pico_modules
    subscription_manager
    clock
    plaintext_transport
)

target_include_directories(
    ${DEMO_NAME_TASKS} PRIVATE 
    "../"
    "${TIF_PATH}"
    ${MQTT_INCLUDE_PUBLIC_DIRS}
    ${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}
    ${SVMQTT_PATH}/platform/transport
    ${SVMQTT_PATH}/platform/clock
    ${SVMQTT_PATH}/logging
    ${SVMQTT_PATH}/subscription_manager
)
