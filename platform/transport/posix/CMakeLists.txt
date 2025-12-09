cmake_minimum_required(VERSION 3.12)

add_library(plaintext_transport plaintext_transport.c sockets_impl.c)

target_include_directories(
    plaintext_transport PRIVATE
    "../../../clogging"
    "../"
)
