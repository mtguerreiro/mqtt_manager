cmake_minimum_required(VERSION 3.12)

# Detect ESP-IDF and force platform name
if(DEFINED ENV{IDF_PATH} OR "$ENV{CMAKE_SYSTEM_NAME}" STREQUAL "ESP-IDF")
    set(PLATFORM_NAME "esp32")
endif()

if( PLATFORM_NAME STREQUAL "posix")
    add_subdirectory(posix)
elseif (PLATFORM_NAME STREQUAL "picow")
    add_subdirectory(picow)
elseif (PLATFORM_NAME STREQUAL "esp32")
    set(SRCS esp32/clock_esp32.c)    
    idf_component_register(
        SRCS            ${SRCS}
        INCLUDE_DIRS    "."
    PRIV_REQUIRES
        esp_timer   # <-- add this line
    )
#add_subdirectory(esp32)
else ()
    message("-- SVMQTT_PLATFORM: platform ${PLATFORM_NAME} not supported for this example.")
endif()

