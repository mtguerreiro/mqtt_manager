cmake_minimum_required(VERSION 3.12)

if( PLATFORM_NAME STREQUAL "posix")
    add_subdirectory(posix)
elseif (PLATFORM_NAME STREQUAL "picow")
    add_subdirectory(picow)
elseif (PLATFORM_NAME STREQUAL "esp32")
    add_subdirectory(esp32)
else ()
    message("-- SVMQTT_PLATFORM: platform ${PLATFORM_NAME} not supported for this example.")
endif()

