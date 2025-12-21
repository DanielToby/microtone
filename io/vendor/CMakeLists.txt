set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build PortAudio and RtMidi as shared libraries" FORCE)

if(${PLATFORM} STREQUAL "linux")
    set(RTMIDI_API_ALSA ON CACHE BOOL "" FORCE)
    set(RTMIDI_API_JACK OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(portaudio)
add_subdirectory(rtmidi)
