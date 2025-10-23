cmake_minimum_required(VERSION 3.20)
project(asciiboard LANGUAGES CXX)

add_subdirectory(vendor)

set(SOURCES
    src/asciiboard/asciiboard.hpp
    src/asciiboard/asciiboard.cpp
    src/asciiboard/main.cpp
    src/asciiboard/processor.hpp
    src/asciiboard/synth_controls.hpp
)

add_executable(asciiboard ${SOURCES})

target_include_directories(asciiboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(asciiboard PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(asciiboard PRIVATE
    common
    io
    synth
    ftxui::component
    ftxui::dom
    ftxui::screen
)
