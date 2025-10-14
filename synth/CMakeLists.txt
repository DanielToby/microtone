cmake_minimum_required(VERSION 3.29)

add_subdirectory(vendor)

add_library(synth STATIC)

set_target_properties(synth PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(synth PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(HEADERS
    src/synth/audio_buffer.hpp
    src/synth/envelope.hpp
    src/synth/filter.hpp
    src/synth/low_frequency_oscillator.hpp
    src/synth/oscillator.hpp
    src/synth/synthesizer.hpp
    src/synth/synthesizer_voice.hpp
    src/synth/wavetable.hpp
    src/synth/weighted_wavetable.hpp
)

set(SOURCES
    src/synth/envelope.cpp
    src/synth/filter.cpp
    src/synth/low_frequency_oscillator.cpp
    src/synth/oscillator.cpp
    src/synth/synthesizer.cpp
    src/synth/synthesizer_voice.cpp
)

target_sources(synth PUBLIC ${HEADERS} ${SOURCES})

if(APPLE)
    set(PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES
        "-framework AudioToolbox"
        "-framework Carbon"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreServices"
        "-framework CoreMIDI"
    )
elseif(WIN32)
    set(PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES "")
endif()

target_link_libraries(synth PUBLIC
    common
    io
    portaudio
)
