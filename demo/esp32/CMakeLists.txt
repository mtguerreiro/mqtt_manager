# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

#cmake_path(SET SVMQTT_PATH NORMALIZE "/home/marco/projects/svmqtt/")
#message("-- SVMQTT_PATH IS ${SVMQTT_PATH}")
#set(EXTRA_COMPONENT_DIRS "${SVMQTT_PATH}/platform/clock/esp32")  # Register external clock component

set(PLATFORM_NAME "esp32")

set(COMPILER_WARNINGS_AS_ERRORS OFF)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)

set(EXTRA_COMPONENT_DIRS
    /home/marco/projects/mqtt_manager
    /home/marco/projects/mqtt_manager/platform/clock
    /home/marco/projects/mqtt_manager/platform/transport
    #/home/marco/projects/mqtt_manager/platform/clock/esp32
    #/home/marco/projects/mqtt_manager/platform/transport/esp32
    #/home/marco/projects/mqtt_manager/logging
)

project(wifi_station)

add_compile_options(-Wno-error)

# Include mqtt manager
#add_subdirectory(${SVMQTT_PATH} "${PROJECT_BINARY_DIR}/mqtt_manager")

#add_executable(
#    wifi_station
#)

#target_link_libraries(
#    wifi_station
#    PRIVATE
#    mqtt_manager
#)

#target_include_directories(
#    wifi_station
#    PUBLIC
#    ${CMAKE_CURRENT_LIST_DIR}
#    ${SVMQTT_PATH}/logging
#)
