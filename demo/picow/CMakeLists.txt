cmake_minimum_required(VERSION 3.12)

set(PLATFORM_NAME "pico")

set(PICO_BOARD pico_w)
cmake_path(SET PROJECTS_PATH NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(FREERTOS_KERNEL_PATH "${PROJECTS_PATH}/freertos")
set(MDRIVERS_PATH "${PROJECTS_PATH}/mdrivers")
set(MQTT_MNG_PATH "${PROJECTS_PATH}/mqtt_manager")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(main C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_DEOPTIMIZED_DEBUG 1)
set(WIFI_SSID "--")
set(WIFI_PASSWORD "--")
set(USE_NW_TCP_SERVER 1)

# Initialize the SDK
pico_sdk_init()
add_compile_options(-Wall -Wextra -O2)

include_directories(${PROJECT_SOURCE_DIR})  # Needed so FreeRTOS finds FreeRTOS config
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Include platform
add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

add_subdirectory(tasks)

add_executable(main picow_demo.c)

target_include_directories(
    main
    PUBLIC
    ./
    ${PROJECT_SOURCE_DIR}
    ${MDRIVERS_PATH}
    ${MQTT_MNG_PATH}/clogging
)

target_link_libraries(
    main
    pico_cyw43_arch_lwip_sys_freertos
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    tasks
    mdrivers
    mqtt_manager
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)
