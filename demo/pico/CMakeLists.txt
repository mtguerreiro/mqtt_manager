cmake_minimum_required(VERSION 3.12)

set(PLATFORM_NAME "pico")

cmake_path(SET PROJECTS_PATH NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(FREERTOS_KERNEL_PATH "${PROJECTS_PATH}/freertos")
set(MDRIVERS_PATH "${PROJECTS_PATH}/mdrivers")
set(MQTT_MNG_PATH "${PROJECTS_PATH}/mqtt_manager")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_DEOPTIMIZED_DEBUG 0)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall -O2)

include_directories(${PROJECT_SOURCE_DIR}) # Needed so FreeRTOS finds FreeRTOS config
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Libraries
add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

# Add projects
add_subdirectory(main)
