cmake_minimum_required(VERSION 3.29)

add_subdirectory(vendor)

add_library(common STATIC)

set_target_properties(common PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/common/exception.cpp
    src/common/exception.hpp
    src/common/log.cpp
    src/common/log.hpp
    src/common/midi_handle.hpp
    src/common/mutex_protected.hpp
    src/common/ring_buffer.hpp
)

target_sources(common PUBLIC ${SOURCES})

target_link_libraries(common PUBLIC
    fmt
    spdlog
)
