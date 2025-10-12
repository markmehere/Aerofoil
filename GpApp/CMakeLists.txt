add_library(
    GpApp
    STATIC
    # consider an explicit listing rather use this monofile - for incremental builds and such
    GpApp_Combined.cpp
)

target_include_directories(
    GpApp
    PUBLIC
    ../Common
    ../GpCommon
    ../PortabilityLayer
)

target_compile_definitions(
    GpApp
    PUBLIC
    GP_DEBUG_CONFIG=0
    GP_ZLIB_BUILTIN=1
)
