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

add_compile_options(-Wall -Wextra -O2)

include_directories(${PROJECT_SOURCE_DIR}) # Needed so FreeRTOS finds FreeRTOS config
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Libraries
add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

# Application
add_executable(main main.c)
add_subdirectory(tasks)

target_link_libraries(
    main
    tasks
    pico_stdlib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4 mdrivers
)

target_include_directories(
    main
    PRIVATE
    ${MDRIVERS_PATH}
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)
