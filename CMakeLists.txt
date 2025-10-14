cmake_minimum_required(VERSION 3.29)
project(microtone)

if(APPLE)
    set (PLATFORM "macos")
elseif(WIN32)
    set (PLATFORM "windows")
endif()

add_subdirectory(common)
add_subdirectory(demo)
add_subdirectory(io)
add_subdirectory(synth)
