set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build PortAudio, RtMidi, SpdLog as shared libraries" FORCE)
set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external format in Spdlog" FORCE)

add_subdirectory(rtmidi)
add_subdirectory(portaudio)
add_subdirectory(spdlog)
