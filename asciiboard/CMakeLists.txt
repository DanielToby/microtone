cmake_minimum_required(VERSION 3.29)
project(asciiboard LANGUAGES CXX)

add_subdirectory(vendor)

add_executable(asciiboard)

set_target_properties(asciiboard PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(asciiboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(HEADERS
    include/asciiboard/asciiboard.hpp
    include/asciiboard/synth_controls.hpp
)

set(SOURCES
    src/asciiboard.cpp
    src/main.cpp
)

target_sources(asciiboard PRIVATE ${HEADERS} ${SOURCES})

target_link_libraries(asciiboard PRIVATE
    microtone
    fmt
    ftxui::component
    ftxui::dom
    ftxui::screen
)

