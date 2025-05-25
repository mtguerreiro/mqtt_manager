cmake_minimum_required(VERSION 3.12)

set(PLATFORM_NAME "picow")

cmake_path(SET PROJECTS_PATH NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(FREERTOS_KERNEL_PATH "${PROJECTS_PATH}/freertos")
set(MDRIVERS_PATH "${PROJECTS_PATH}/mdrivers")
set(MQTT_MNG_PATH "${PROJECTS_PATH}/mqtt_manager")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)
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

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()
add_compile_options(
    -Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
    -O2
)

# Include platform
add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

add_subdirectory(tasks)

add_executable(main picow_demo.c)

target_include_directories(
    main
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${MDRIVERS_PATH}
    ${MQTT_MNG_PATH}/logging
)

target_link_libraries(
    main
    tasks
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    mdrivers
    mqtt_manager
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)
