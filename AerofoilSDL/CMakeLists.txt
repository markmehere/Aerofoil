set(SOURCES
    GpAudioDriver_SDL2.cpp
    GpDisplayDriver_SDL_GL2.cpp
    GpInputDriver_SDL_Gamepad.cpp
    ShaderCode/CopyQuadP.cpp
    ShaderCode/DrawQuadPaletteP.cpp
    ShaderCode/DrawQuad32P.cpp
    ShaderCode/DrawQuadV.cpp
    ShaderCode/ScaleQuadP.cpp
)

add_library(
    AerofoilSDL
    STATIC
    ${SOURCES}
)

get_filename_component(REAL_SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2 REALPATH)

target_include_directories(
    AerofoilSDL
    PUBLIC
    ../Common
    ../GpCommon
    ../GpShell
    ../PortabilityLayer
    ../AerofoilPortable
    ${REAL_SDL2_SOURCE_DIR}/include
)

target_compile_definitions(
    AerofoilSDL
    PUBLIC
    GP_DEBUG_CONFIG=0
)
