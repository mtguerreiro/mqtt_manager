cmake_minimum_required(VERSION 3.12)

add_library(subscription_manager mqttmng.c mqtt_subscription_manager.c)

target_link_libraries(
    subscription_manager
    PRIVATE
    clock_posix
    plaintext_transport
)

target_include_directories(subscription_manager PRIVATE
                           "${PROJECT_SOURCE_DIR}"
                           "${MQTT_INCLUDE_PUBLIC_DIRS}"
                           "${SVMQTT_PATH}/logging"
                           "${SVMQTT_PATH}/platform/transport"
                           "${SVMQTT_PATH}/platform/clock"
)
