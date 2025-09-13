
list(APPEND USER_COMPILE_DEFINITIONS MQTT_DO_NOT_USE_CUSTOM_CONFIG)

include( "${MQTT_MNG_PATH}/coreMQTT/mqttFilePaths.cmake" )

# Headers
list(APPEND USER_INCLUDE_DIRECTORIES
    "${MQTT_INCLUDE_PUBLIC_DIRS}"
    "${MQTT_MNG_PATH}/platform/transport/"
    "${MQTT_MNG_PATH}/platform/transport/zynq/"
    "${MQTT_MNG_PATH}/platform/clock/"
    "${MQTT_MNG_PATH}/platform/clock/zynq"
    "${MQTT_MNG_PATH}/clogging/"
    "${MQTT_MNG_PATH}/mqtt_manager/"
)

# Sources
list(APPEND USER_COMPILE_SOURCES
    "${MQTT_MNG_PATH}/platform/transport/zynq/plaintext_transport.c"
    "${MQTT_MNG_PATH}/platform/clock/zynq/clock_zynq.c"
    )

list(APPEND USER_COMPILE_SOURCES
    "${MQTT_MNG_PATH}/mqtt_manager/mqttmng.c"
    "${MQTT_MNG_PATH}/mqtt_manager/mqtt_subscription_manager.c"
)

list(APPEND USER_COMPILE_SOURCES
    ${MQTT_SOURCES}
    ${MQTT_SERIALIZER_SOURCES}
)
