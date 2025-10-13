# Ensure static libs
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Avoid building examples/tests/docs from FTXUI
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory(ftxui)
