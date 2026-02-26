cmake_minimum_required(VERSION 3.29)
project(microtone)

if(APPLE)
    set (PLATFORM "macos")
elseif(WIN32)
    set (PLATFORM "windows")
elseif(UNIX)
    set (PLATFORM "linux")
endif()

option(ENABLE_GPIO_CONTROL "Enable GPIO controls" OFF)

if (ENABLE_GPIO_CONTROL AND NOT ${PLATFORM} STREQUAL "linux")
    message(FATAL_ERROR "GPIO control is only supported on linux.")
endif()

add_subdirectory(common)
add_subdirectory(demo)
add_subdirectory(io)
add_subdirectory(synth)
