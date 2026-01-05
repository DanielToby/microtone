cmake_minimum_required(VERSION 3.29)

add_library(synth STATIC)

set_target_properties(synth PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(synth PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/synth/adsr.hpp
    src/synth/audio_pipeline.cpp
    src/synth/audio_pipeline.hpp
    src/synth/envelope.hpp
    src/synth/filter.hpp
    src/synth/instrument.hpp
    src/synth/low_frequency_oscillator.hpp
    src/synth/oscillator.hpp
    src/synth/synthesizer.cpp
    src/synth/synthesizer.hpp
    src/synth/voice.hpp
    src/synth/wave_table.hpp
)

target_sources(synth PUBLIC ${SOURCES})

target_link_libraries(synth PUBLIC
    common
)
