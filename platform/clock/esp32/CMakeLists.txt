idf_component_register(
    SRCS "clock_esp32.c"
    INCLUDE_DIRS "."
)

#set(COMPONENT_NAME "clock")

#idf_component_register(
#    SRCS "clock_esp32.c"
#    INCLUDE_DIRS "../"  # Or the appropriate include folder if you have headers
#    REQUIRES esp_timer  # Automatically links the dependencies (like esp_common, esp_hw_support, etc.)
#)

#cmake_minimum_required(VERSION 3.12)
#
#add_library(clock clock_esp32.c)
#
#target_include_directories(
#   clock PRIVATE
#    "${IDF_PATH}/components/esp_timer/include"
#    "${IDF_PATH}/components/esp_common/include"
#    "${IDF_PATH}/components/esp_hw_support/include"
#    "../"
#    "${CMAKE_BINARY_DIR}/config"
#
#)
#
#target_link_libraries(clock PRIVATE
#    esp_timer
#    esp_common
#    esp_hw_support
#    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
#)
#
