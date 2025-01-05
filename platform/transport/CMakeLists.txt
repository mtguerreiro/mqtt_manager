cmake_minimum_required(VERSION 3.12)

add_library(plaintext_transport plaintext_transport.c)

if( PLATFORM_NAME STREQUAL "posix")
    add_subdirectory(posix)
elseif (CMAKE_SYSTEM_NAME STREQUAL "picow")
    add_subdirectory(picow)
else ()
    message("-- SVMQTT_PLATFORM: platform ${PLATFORM_NAME} not supported for this example.")
endif()

target_link_libraries(plaintext_transport
    sockets_impl
)

target_include_directories(
    plaintext_transport PRIVATE
    "${SVMQTT_PATH}/logging"
)
