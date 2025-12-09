cmake_minimum_required(VERSION 3.12)

if( DEFINED ENV{IDF_PATH} OR "$ENV{CMAKE_SYSTEM_NAME}" STREQUAL "ESP-IDF" )
    set(PLATFORM_NAME "esp32")
elseif( NOT DEFINED PLATFORM_NAME )
    set(PLATFORM_NAME "posix")
    message("-- MQTT_MNG: Setting platform to posix because none was specified.")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_NAME}.cmake)
