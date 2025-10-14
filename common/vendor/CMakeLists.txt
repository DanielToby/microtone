set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "Use external format in Spdlog" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build SpdLog as shared library" FORCE)

add_subdirectory(fmt)
add_subdirectory(spdlog)
