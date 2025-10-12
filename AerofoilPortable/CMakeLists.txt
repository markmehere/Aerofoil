add_library(
    AerofoilPortable
    STATIC
    GpAllocator_C.cpp
    GpThreadEvent_Cpp11.cpp
    GpSystemServices_POSIX.cpp
)

target_include_directories(
    AerofoilPortable
    PUBLIC
    ../Common
    ../GpCommon
    ../GpShell
    ../PortabilityLayer
)

target_compile_definitions(
    AerofoilPortable
    PUBLIC
    GP_DEBUG_CONFIG=0
)
