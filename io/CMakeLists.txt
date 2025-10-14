cmake_minimum_required(VERSION 3.29)

add_subdirectory(vendor)

add_library(io STATIC)

set_target_properties(io PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(HEADERS
    src/io/midi_input.hpp
)

set(SOURCES
    src/io/midi_input.cpp
)

target_sources(io PUBLIC ${HEADERS} ${SOURCES})

target_link_libraries(io PUBLIC
    common
    rtmidi
)
