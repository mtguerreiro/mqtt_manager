cmake_minimum_required(VERSION 3.12)

if( PLATFORM_NAME STREQUAL "posix")
    add_subdirectory(posix)
elseif (CMAKE_SYSTEM_NAME STREQUAL "picow")
    add_subdirectory(picow)
else ()
    message("-- SVMQTT_PLATFORM: platform ${PLATFORM_NAME} not supported for this example.")
endif()

target_include_directories(
    plaintext_transport PRIVATE
    "${SVMQTT_PATH}/logging"
)
