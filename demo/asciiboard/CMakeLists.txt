cmake_minimum_required(VERSION 3.20)
project(asciiboard LANGUAGES CXX)

add_subdirectory(vendor)

set(SOURCES
    src/asciiboard/asciiboard.hpp
    src/asciiboard/asciiboard.cpp
    src/asciiboard/demo_midi_generator.hpp
    src/asciiboard/main.cpp
    src/asciiboard/render_loop.hpp
    src/asciiboard/state.hpp
    src/asciiboard/components/effects_controls.hpp
    src/asciiboard/components/envelope_controls.hpp
    src/asciiboard/components/envelope_graph.hpp
    src/asciiboard/components/graph.hpp
    src/asciiboard/components/info_message.hpp
    src/asciiboard/components/oscillator_controls.hpp
    src/asciiboard/components/oscilloscope.hpp
    src/asciiboard/components/piano_roll.hpp
)

add_executable(asciiboard ${SOURCES})

target_include_directories(asciiboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(asciiboard PROPERTIES
    CXX_STANDARD 20
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

if(${PLATFORM} STREQUAL "linux")
    target_link_libraries(asciiboard PRIVATE atomic)
endif()
