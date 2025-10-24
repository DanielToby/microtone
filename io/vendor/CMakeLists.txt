set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build PortAudio and RtMidi as shared libraries" FORCE)

add_subdirectory(portaudio)
add_subdirectory(rtmidi)
