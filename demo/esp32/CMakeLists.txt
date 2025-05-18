# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

set(PLATFORM_NAME "esp32")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)

cmake_path(SET MQTT_MNG_PATH NORMALIZE "${CMAKE_SOURCE_DIR}/../../")
message("-- MQTT_MNG_PATH IS ${MQTT_MNG_PATH}")

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/tasks
    ${MQTT_MNG_PATH}
    ${MQTT_MNG_PATH}/platform/clock
    ${MQTT_MNG_PATH}/platform/transport
)

project(wifi_station)


