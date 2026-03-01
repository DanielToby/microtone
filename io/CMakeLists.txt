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
    src/io/gpio_components.hpp
    src/io/gpio_input.cpp
    src/io/gpio_input.hpp
    src/io/midi_input_stream.cpp
    src/io/midi_input_stream.hpp
    src/io/midi_message.hpp
)

target_sources(io PUBLIC ${SOURCES})

target_link_libraries(io
    PUBLIC
        common
    PRIVATE
        portaudio
        rtmidi
)

target_compile_definitions(io
    PUBLIC
        $<$<BOOL:${ENABLE_GPIO_CONTROL}>:ENABLE_GPIO_CONTROL>
)

if(ENABLE_GPIO_CONTROL)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GPIODCXX REQUIRED libgpiodcxx>=2.0)
    target_include_directories(io PRIVATE ${GPIODCXX_INCLUDE_DIRS})
    target_link_libraries(io PRIVATE ${GPIODCXX_LIBRARIES})
endif()
