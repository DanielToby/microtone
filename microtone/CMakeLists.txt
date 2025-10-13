cmake_minimum_required(VERSION 3.29)
project(microtone LANGUAGES CXX)

add_subdirectory(vendor)

add_library(microtone STATIC)

set_target_properties(microtone PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(microtone PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(HEADERS
    src/microtone/microtone_platform.hpp
    src/microtone/exception.hpp
    src/microtone/midi_input.hpp
    src/microtone/synthesizer/audio_buffer.hpp
    src/microtone/synthesizer/envelope.hpp
    src/microtone/synthesizer/filter.hpp
    src/microtone/synthesizer/low_frequency_oscillator.hpp
    src/microtone/synthesizer/oscillator.hpp
    src/microtone/synthesizer/synthesizer.hpp
    src/microtone/synthesizer/synthesizer_voice.hpp
    src/microtone/synthesizer/wavetable.hpp
    src/microtone/synthesizer/weighted_wavetable.hpp
    src/microtone/log.hpp
)

set(SOURCES
    src/microtone/microtone_platform.cpp
    src/microtone/exception.cpp
    src/microtone/log.cpp
    src/microtone/midi_input.cpp
    src/microtone/synthesizer/envelope.cpp
    src/microtone/synthesizer/filter.cpp
    src/microtone/synthesizer/low_frequency_oscillator.cpp
    src/microtone/synthesizer/oscillator.cpp
    src/microtone/synthesizer/synthesizer.cpp
    src/microtone/synthesizer/synthesizer_voice.cpp
)

target_sources(microtone PUBLIC ${HEADERS} ${SOURCES})

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

target_link_libraries(microtone PUBLIC
    fmt
    portaudio
    rtmidi
    spdlog
    ${PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES}
)
