cmake_minimum_required(VERSION 3.15)

set(PLATFORM_NAME "posix")

cmake_path(SET MQTT_MNG_PATH NORMALIZE "${CMAKE_SOURCE_DIR}/../../")
message("-- MQTT_MNG_PATH IS ${MQTT_MNG_PATH}")

set(DEMO_NAME "main")
set(DEMO_FILE "${DEMO_NAME}.c")
message("-- Demo file is ${DEMO_FILE}")

project(coremqtt_example)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_definitions(MQTT_DO_NOT_USE_CUSTOM_CONFIG)
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -O2
        )

add_subdirectory(${MQTT_MNG_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")
add_subdirectory(threads)

add_executable(
    ${DEMO_NAME}
    "${DEMO_FILE}"
)

target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
    threads
    mqtt_manager
)

