cmake_minimum_required(VERSION 3.12)

idf_component_register(
    SRCS esp32/clock_esp32.c
    INCLUDE_DIRS "." ../../clogging
    PRIV_REQUIRES esp_timer
)
