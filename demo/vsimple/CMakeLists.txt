cmake_minimum_required(VERSION 3.15)

project(coremqtt_example)

cmake_path(SET SVMQTT_PATH NORMALIZE "${PROJECT_SOURCE_DIR}/../../")
message("-- SVMQTT_PATH IS ${SVMQTT_PATH}")

set(DEMO_NAME "main")
set(DEMO_FILE "${DEMO_NAME}.c")
message("-- Demo file is ${DEMO_FILE}")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Include MQTT library's source and header path variables.
include( ${SVMQTT_PATH}/coreMQTT/mqttFilePaths.cmake )

# Include backoffAlgorithm library file path configuration.
include( ${SVMQTT_PATH}/backoffAlgorithm/backoffAlgorithmFilePaths.cmake )

# Include platform
add_subdirectory(${SVMQTT_PATH}/platform "${PROJECT_BINARY_DIR}/platform")

# Include subscription manager
add_subdirectory(${SVMQTT_PATH}/subscription_manager "${PROJECT_BINARY_DIR}/subscription_manager")

add_compile_definitions(MQTT_DO_NOT_USE_CUSTOM_CONFIG)
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -O2
        )

# Demo target.
add_executable(
    ${DEMO_NAME}
        "${DEMO_FILE}"
        ${MQTT_SOURCES}
        ${MQTT_SERIALIZER_SOURCES}
        ${BACKOFF_ALGORITHM_SOURCES}
)

target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        subscription_manager
        clock_posix
        plaintext_posix
)
target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${MQTT_INCLUDE_PUBLIC_DIRS}
        ${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}
        ${CMAKE_CURRENT_LIST_DIR}
        ${SVMQTT_PATH}/platform/transport
        ${SVMQTT_PATH}/platform/clock
        ${SVMQTT_PATH}/logging
        ${SVMQTT_PATH}/subscription_manager
)
