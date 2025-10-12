add_library(
    GpShell
    STATIC
    # consider an explicit listing rather use this monofile - for incremental builds and such
    GpShell_Combined.cpp
)

get_filename_component(REAL_SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2 REALPATH)

target_include_directories(
    GpShell
    PUBLIC
    ../Common
    ../GpCommon
    ../PortabilityLayer
    ${REAL_SDL2_SOURCE_DIR}/include
)

target_compile_definitions(
    GpShell
    PUBLIC
    GP_DEBUG_CONFIG=0
)
