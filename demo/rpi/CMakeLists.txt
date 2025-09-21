cmake_minimum_required(VERSION 3.15)

set(PLATFORM_NAME "rpi")

cmake_path(SET PROJECTS_PATH NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(MDRIVERS_PATH "${PROJECTS_PATH}/mdrivers")
set(MQTT_MNG_PATH "${PROJECTS_PATH}/mqtt_manager")

message("-- MQTT_MNG_PATH IS ${MQTT_MNG_PATH}")

project(posix_mqttmng_example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic -O2)

add_subdirectory(${MDRIVERS_PATH} "${PROJECT_BINARY_DIR}/mdrivers")
add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")
add_subdirectory(threads)

add_executable(
    posix_mqttmng_example
    main.c
)

target_link_libraries(
    posix_mqttmng_example
    PRIVATE
    threads
    mqtt_manager
)
