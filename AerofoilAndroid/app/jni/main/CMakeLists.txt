add_library(
    main
    SHARED
    GpMain_SDL_Android.cpp
    GpSystemServices_Android.cpp
    GpFileSystem_Android.cpp
)

get_filename_component(REAL_SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../SDL2 REALPATH)

target_include_directories(
    main
    PUBLIC
    ${REAL_SDL2_SOURCE_DIR}/include
    ../GpShell
    ../GpCommon
    ../AerofoilPortable
    ../AerofoilSDL
    ../Common
    ../PortabilityLayer
)

target_compile_definitions(
    main
    PUBLIC
    GP_DEBUG_CONFIG=0
)

target_link_libraries(
    main
    GpShell
    AerofoilPortable
    AerofoilSDL
    GpApp
    PortabilityLayer
    SDL2
    GLESv1_CM
    GLESv2
    log
)