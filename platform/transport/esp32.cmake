cmake_minimum_required(VERSION 3.12)

idf_component_register(
    SRCS esp32/plaintext_transport.c esp32/sockets_impl.c
    INCLUDE_DIRS "." ../../clogging
)

