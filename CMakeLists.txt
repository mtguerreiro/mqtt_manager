cmake_minimum_required(VERSION 3.12)

add_compile_definitions(MQTT_DO_NOT_USE_CUSTOM_CONFIG)

add_subdirectory(platform)

add_library(mqtt_manager)

# Include MQTT library's source and header path variables.
include( coreMQTT/mqttFilePaths.cmake )

# MQTT manager
target_sources(
    mqtt_manager
    PRIVATE
    mqtt_manager/mqttmng.c
    mqtt_manager/mqtt_subscription_manager.c
)

# Core MQTT
target_sources(
    mqtt_manager
    PRIVATE
    ${MQTT_SOURCES}
    ${MQTT_SERIALIZER_SOURCES}
)

target_link_libraries(
    mqtt_manager
    PRIVATE
    clock
    plaintext_transport
)

target_include_directories(
    mqtt_manager
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${MQTT_INCLUDE_PUBLIC_DIRS}
    platform/transport/
    platform/clock/
    logging/
    mqtt_manager/
)

target_include_directories(
    mqtt_manager
    PUBLIC
    ${MQTT_INCLUDE_PUBLIC_DIRS}
    mqtt_manager/
)
