cmake_minimum_required(VERSION 3.29)
project(asciiboard LANGUAGES CXX)

add_executable(asciiboard)

set_target_properties(asciiboard PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_compile_definitions(asciiboard PRIVATE
    FMT_HEADER_ONLY
)

if(APPLE)
    set(FTXUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ftxui-2.0.0/include/macos)
elseif(WIN32)
    set(FTXUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ftxui-2.0.0/include/windows)
endif()

target_include_directories(asciiboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/fmt-8.0.1/include
    ${FTXUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/include
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

if(APPLE)
    target_compile_definitions(asciiboard PRIVATE __MACOSX_CORE__)

    # Library search paths
    target_link_directories(asciiboard PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ftxui-2.0.0/lib/macos
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/rtmidi-5.0.0/lib/macos
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/portaudio-19.7.0/lib/macos
    )

    # Link libraries
    target_link_libraries(asciiboard PRIVATE
        ftxui-component
        ftxui-dom
        ftxui-screen
        microtone
        rtmidi
        portaudio
        "-framework AudioToolbox"
        "-framework Carbon"
        "-framework CoreAudio"
        "-framework CoreFoundation"
        "-framework CoreServices"
        "-framework CoreMIDI"
    )

elseif(WIN32)
    target_compile_definitions(asciiboard PRIVATE __WINDOWS_MM__)

    # Set build directory for Microtone dependency
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(DEST_DIR "debug")
    else()
        set(DEST_DIR "release")
    endif()

    # Library search paths
    target_link_directories(asciiboard PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/ftxui-2.0.0/lib/windows
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/${DEST_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/rtmidi-5.0.0/lib/windows
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/portaudio-19.7.0/lib/windows
    )

    # Link libraries
    target_link_libraries(asciiboard PRIVATE
        ftxui-component.lib
        ftxui-dom.lib
        ftxui-screen.lib
        Microtone.lib
        rtmidi.lib
        portaudio_x64.lib
    )

    # Ensure necessary DLLs are copied after build
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    add_custom_command(TARGET asciiboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/rtmidi-5.0.0/lib/windows/rtmidi.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

    add_custom_command(TARGET asciiboard POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../microtone/vendor/portaudio-19.7.0/lib/windows/portaudio_x64.dll
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
