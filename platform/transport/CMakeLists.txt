cmake_minimum_required(VERSION 3.12)

add_library(plaintext_posix plaintext_posix.c sockets_posix.c)

target_include_directories(plaintext_posix PRIVATE
                           "${PROJECT_SOURCE_DIR}"
                           "${SVMQTT_PATH}/logging"
)
