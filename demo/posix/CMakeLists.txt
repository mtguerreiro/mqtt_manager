cmake_minimum_required(VERSION 3.15)

project(coremqtt_example)
set(PLATFORM_NAME "posix")

cmake_path(SET SVMQTT_PATH NORMALIZE "${PROJECT_SOURCE_DIR}/../../")
message("-- SVMQTT_PATH IS ${SVMQTT_PATH}")

set(DEMO_NAME "main")
set(DEMO_FILE "${DEMO_NAME}.c")
message("-- Demo file is ${DEMO_FILE}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_compile_definitions(MQTT_DO_NOT_USE_CUSTOM_CONFIG)
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -O2
        )

add_subdirectory(${SVMQTT_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")
add_subdirectory(threads)

# Demo target.
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

target_include_directories(
    ${DEMO_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${SVMQTT_PATH}/logging
)
