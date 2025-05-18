cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

set(DEMO_NAME "picow_demo")
set(DEMO_NAME_TASKS "${DEMO_NAME}_tasks")
set(DEMO_FILE "${DEMO_NAME}.c")
set(PLATFORM_NAME "picow")
message("-- Demo file is ${DEMO_FILE}")

# Pull in FreeRTOS
set(FREERTOS_KERNEL_PATH "/home/marco/projects/freertos")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(${DEMO_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_DEOPTIMIZED_DEBUG 1)
set(PICO_BOARD pico_w)
set(WIFI_SSID "--")
set(WIFI_PASSWORD "--")
set(USE_NW_TCP_SERVER 1)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

message("-- Raspberry Pi Pico SDK version is ${PICO_SDK_VERSION_STRING}")

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})
set(TIF_PATH "/home/marco/projects")
message("-- TIF path is ${TIF_PATH}")

# Initialize the SDK
pico_sdk_init()
#include(example_auto_set_url.cmake)

add_compile_definitions(MQTT_DO_NOT_USE_CUSTOM_CONFIG)
add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
    -O2
)

# Include platform
cmake_path(SET MQTT_MNG_PATH NORMALIZE "${PROJECT_SOURCE_DIR}/../../")
message("-- MQTT_MNG_PATH IS ${MQTT_MNG_PATH}")

# Include subscription manager
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

# Libraries
add_subdirectory(${TIF_PATH}/tif "${PROJECT_BINARY_DIR}/tif")
add_subdirectory(tasks)

add_executable(
    ${DEMO_NAME}
    "${DEMO_NAME}.c"
)

target_include_directories(${DEMO_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        "${TIF_PATH}"
        ${MQTT_MNG_PATH}/logging
        )

target_link_libraries(
    ${DEMO_NAME} 
    ${DEMO_NAME_TASKS}
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib
    hw_pico_modules
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    mqtt_manager
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${DEMO_NAME} 1)
pico_enable_stdio_uart(${DEMO_NAME} 0)

pico_add_extra_outputs(${DEMO_NAME})
