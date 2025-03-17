cmake_minimum_required(VERSION 3.29)
project(SynthesizerDemo)

if(APPLE)
    set (PLATFORM "macos")
elseif(WIN32)
    set (PLATFORM "windows")
endif()

# Add subdirectories
add_subdirectory(microtone)
add_subdirectory(asciiboard)
