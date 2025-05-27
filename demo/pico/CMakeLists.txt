cmake_minimum_required(VERSION 3.12)

set(PLATFORM_NAME "pico")

cmake_path(SET PROJECTS_PATH NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(FREERTOS_KERNEL_PATH "${PROJECTS_PATH}/freertos")
set(MDRIVERS_PATH "${PROJECTS_PATH}/mdrivers")
set(MQTT_MNG_PATH "${PROJECTS_PATH}/mqtt_manager")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_DEOPTIMIZED_DEBUG 0)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()
include(example_auto_set_url.cmake)

add_compile_options(-Wall -O2)

# Libraries
add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

# Add projects
add_subdirectory(main)
