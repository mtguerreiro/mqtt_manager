cmake_minimum_required(VERSION 3.12)

add_library(sockets_impl sockets_impl.c)

target_include_directories(
    sockets_impl PRIVATE
    "${SVMQTT_PATH}/logging"
    "../"
)
