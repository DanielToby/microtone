cmake_minimum_required(VERSION 3.29)
project(microtone LANGUAGES CXX)

add_library(microtone STATIC)

set_target_properties(microtone PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_definitions(microtone PUBLIC
    FMT_HEADER_ONLY
    SPDLOG_FMT_EXTERNAL
)

target_include_directories(microtone PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/portaudio-19.7.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rtmidi-5.0.0/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog-1.9.0/include
    ${CMAKE_SOURCE_DIR}/vendor/fmt-8.0.1/include
)

set(HEADERS
    include/microtone/microtone_platform.hpp
    include/microtone/exception.hpp
    include/microtone/midi_input.hpp
    include/microtone/synthesizer/audio_buffer.hpp
    include/microtone/synthesizer/envelope.hpp
    include/microtone/synthesizer/filter.hpp
    include/microtone/synthesizer/low_frequency_oscillator.hpp
    include/microtone/synthesizer/oscillator.hpp
    include/microtone/synthesizer/synthesizer.hpp
    include/microtone/synthesizer/synthesizer_voice.hpp
    include/microtone/synthesizer/wavetable.hpp
    include/microtone/synthesizer/weighted_wavetable.hpp
    src/log.hpp
)

set(SOURCES
    src/microtone_platform.cpp
    src/exception.cpp
    src/log.cpp
    src/midi_input.cpp
    src/synthesizer/envelope.cpp
    src/synthesizer/filter.cpp
    src/synthesizer/low_frequency_oscillator.cpp
    src/synthesizer/oscillator.cpp
    src/synthesizer/synthesizer.cpp
    src/synthesizer/synthesizer_voice.cpp
)

target_sources(microtone PUBLIC ${HEADERS} ${SOURCES})

if(APPLE)
    target_compile_definitions(microtone PUBLIC __MACOSX_CORE__) # < Required by portaudio.
    set(PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES
        "-framework AudioToolbox"
        "-framework Carbon"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreServices"
        "-framework CoreMIDI"
    )
elseif(WIN32)
    target_compile_definitions(microtone PUBLIC __WINDOWS_MM__) # < Required by portaudio.
    set(PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES "")
endif()

target_link_libraries(microtone PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rtmidi-5.0.0/lib/${PLATFORM}/librtmidi.dylib
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/portaudio-19.7.0/lib/${PLATFORM}/libportaudio.a
    ${PLATFORM_SPECIFIC_AUDIO_DEPENDENCIES}
)
