cmake_minimum_required(VERSION 3.12)

add_library(tasks
    task_blink.c
    task_wifi_init.c
    task_mqtt_mng.c
    task_temperature.c
    task_led.c
)

target_compile_definitions(tasks PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
)

target_link_libraries(tasks
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    mdrivers
    mqtt_manager
)

target_include_directories(
    tasks
    PRIVATE 
    "../"
    "${MDRIVERS_PATH}"
    "${MQTT_MNG_PATH}/logging"
)
