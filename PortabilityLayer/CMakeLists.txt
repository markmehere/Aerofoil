add_library(
    PortabilityLayer
    STATIC
    # consider an explicit listing rather use this monofile - for incremental builds and such
    PortabilityLayer_Combined.cpp
)

target_include_directories(
    PortabilityLayer
    PUBLIC
    ./
    ../GpCommon
    ../Common
    ../rapidjson/include
    ../MacRomanConversion
    ../stb
)

target_compile_definitions(
    PortabilityLayer
    PUBLIC
    GP_DEBUG_CONFIG=0
    GP_ZLIB_BUILTIN=1
)

target_link_libraries(
    PortabilityLayer
    PUBLIC
    MacRomanConversion
    stb
)

target_link_libraries(
    PortabilityLayer
    INTERFACE
    z
)
