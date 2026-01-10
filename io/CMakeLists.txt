cmake_minimum_required(VERSION 3.29)

add_subdirectory(vendor)

add_library(io STATIC)

set_target_properties(io PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(io PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/portaudio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rtmidi
)

set(SOURCES
    src/io/audio_output_stream.cpp
    src/io/audio_output_stream.hpp
    src/io/midi_input_stream.cpp
    src/io/midi_input_stream.hpp
    src/io/midi_message.hpp
)

target_sources(io PUBLIC ${SOURCES})

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

target_link_libraries(io
PUBLIC
    common
PRIVATE
    portaudio
    rtmidi
)
