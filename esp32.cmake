#add_subdirectory(platform)
include(${CMAKE_CURRENT_LIST_DIR}/coreMQTT/mqttFilePaths.cmake)

# ESP-IDF build
idf_component_register(
    SRCS
        mqtt_manager/mqttmng.c
        mqtt_manager/mqtt_subscription_manager.c
        ${MQTT_SOURCES}
        ${MQTT_SERIALIZER_SOURCES}
    INCLUDE_DIRS
        . 
        mqtt_manager
        ${CMAKE_SOURCE_DIR}
        ${MQTT_INCLUDE_PUBLIC_DIRS}
        coreMQTT/source/include
        logging
        platform/transport
        platform/clock
    PRIV_REQUIRES
        clock
        transport
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE MQTT_DO_NOT_USE_CUSTOM_CONFIG)
