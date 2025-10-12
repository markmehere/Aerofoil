# SDL2/CMakeLists.txt
cmake_minimum_required(VERSION 3.18)

project(SDL2)

# ----------------------------------------------
# 1. Collect Source Files (Resolving Soft Links)
# ----------------------------------------------

# Get the REAL physical path of this directory, following the soft link.
get_filename_component(REAL_SDL2_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)

# --- DEBUG --- Print the real and current source directories
message(STATUS "DEBUG: CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "DEBUG: REAL_SDL2_SOURCE_DIR (resolved path) = ${REAL_SDL2_SOURCE_DIR}")
# --- END DEBUG ---

set(CPUFEATURES_LIB_PATH
  "${ANDROID_NDK}/sources/android/cpufeatures"
)

# Define the source directories relative to the REAL path
set(SDL_C_DIRS
  ${REAL_SDL2_SOURCE_DIR}/src
  ${REAL_SDL2_SOURCE_DIR}/src/audio
  ${REAL_SDL2_SOURCE_DIR}/src/audio/android
  ${REAL_SDL2_SOURCE_DIR}/src/audio/dummy
  ${REAL_SDL2_SOURCE_DIR}/src/audio/aaudio
  ${REAL_SDL2_SOURCE_DIR}/src/audio/opensles
  ${REAL_SDL2_SOURCE_DIR}/src/core/android
  ${REAL_SDL2_SOURCE_DIR}/src/cpuinfo
  ${REAL_SDL2_SOURCE_DIR}/src/dynapi
  ${REAL_SDL2_SOURCE_DIR}/src/events
  ${REAL_SDL2_SOURCE_DIR}/src/file
  ${REAL_SDL2_SOURCE_DIR}/src/haptic
  ${REAL_SDL2_SOURCE_DIR}/src/haptic/android
  ${REAL_SDL2_SOURCE_DIR}/src/hidapi
  ${REAL_SDL2_SOURCE_DIR}/src/joystick
  ${REAL_SDL2_SOURCE_DIR}/src/joystick/android
  ${REAL_SDL2_SOURCE_DIR}/src/joystick/hidapi
  ${REAL_SDL2_SOURCE_DIR}/src/joystick/virtual
  ${REAL_SDL2_SOURCE_DIR}/src/loadso/dlopen
  ${REAL_SDL2_SOURCE_DIR}/src/locale
  ${REAL_SDL2_SOURCE_DIR}/src/locale/android
  ${REAL_SDL2_SOURCE_DIR}/src/misc
  ${REAL_SDL2_SOURCE_DIR}/src/misc/android
  ${REAL_SDL2_SOURCE_DIR}/src/power
  ${REAL_SDL2_SOURCE_DIR}/src/power/android
  ${REAL_SDL2_SOURCE_DIR}/src/filesystem/android
  ${REAL_SDL2_SOURCE_DIR}/src/sensor
  ${REAL_SDL2_SOURCE_DIR}/src/sensor/android
  ${REAL_SDL2_SOURCE_DIR}/src/render
  ${REAL_SDL2_SOURCE_DIR}/src/render/direct3d
  ${REAL_SDL2_SOURCE_DIR}/src/render/direct3d11
  ${REAL_SDL2_SOURCE_DIR}/src/render/direct3d12
  ${REAL_SDL2_SOURCE_DIR}/src/render/metal
  ${REAL_SDL2_SOURCE_DIR}/src/render/opengl
  ${REAL_SDL2_SOURCE_DIR}/src/render/opengles
  ${REAL_SDL2_SOURCE_DIR}/src/render/opengles2
  ${REAL_SDL2_SOURCE_DIR}/src/render/psp
  ${REAL_SDL2_SOURCE_DIR}/src/render/software
  ${REAL_SDL2_SOURCE_DIR}/src/render/vita
  ${REAL_SDL2_SOURCE_DIR}/src/stdlib
  ${REAL_SDL2_SOURCE_DIR}/src/thread
  ${REAL_SDL2_SOURCE_DIR}/src/thread/pthread
  ${REAL_SDL2_SOURCE_DIR}/src/timer
  ${REAL_SDL2_SOURCE_DIR}/src/timer/unix
  ${REAL_SDL2_SOURCE_DIR}/src/video
  ${REAL_SDL2_SOURCE_DIR}/src/video/android
  ${REAL_SDL2_SOURCE_DIR}/src/video/yuv2rgb
  ${REAL_SDL2_SOURCE_DIR}/src/test
  ${CPUFEATURES_LIB_PATH}
)

set(SDL_SOURCES)
# Use the REAL path to glob for files. This correctly finds the sources.
foreach(DIR ${SDL_C_DIRS})
  file(GLOB C_FILES "${DIR}/*.c")
  if(C_FILES)
    # --- DEBUG --- Print which directories yield files
    # message(STATUS "DEBUG: Found in ${DIR}: ${C_FILES}")
    # --- END DEBUG ---
    list(APPEND SDL_SOURCES ${C_FILES})
  endif()
endforeach()

# Manually add specific files using the REAL path
list(APPEND SDL_SOURCES
  "${REAL_SDL2_SOURCE_DIR}/src/hidapi/android/hid.cpp"
  "${REAL_SDL2_SOURCE_DIR}/src/atomic/SDL_atomic.c"
  "${REAL_SDL2_SOURCE_DIR}/src/atomic/SDL_spinlock.c"
)

# --- DEBUG --- Check the final total list size before adding the library
list(LENGTH SDL_SOURCES SDL_SOURCES_COUNT)
message(STATUS "DEBUG: Total sources found: ${SDL_SOURCES_COUNT}")
if(SDL_SOURCES_COUNT EQUAL 0)
  message(FATAL_ERROR "FATAL: SDL_SOURCES list is empty. Halting build.")
endif()
# --- END DEBUG ---

# ----------------------------------------------
# 2. Build Targets (Now with a guaranteed source list)
# ----------------------------------------------

# Pass the source list directly at creation. This is cleaner and safer.
add_library(SDL2 SHARED ${SDL_SOURCES})

# --- Properties are applied after creation ---
target_include_directories(SDL2 PUBLIC
  "${ANDROID_NDK}/sources/android/cpufeatures"
  "${REAL_SDL2_SOURCE_DIR}/include"
)

# Compiler Definitions (like -D flags)
target_compile_definitions(SDL2 PUBLIC GL_GLEXT_PROTOTYPES)

# Compiler Options/Flags (like -W flags)
target_compile_options(SDL2 PRIVATE
  -Wall -Wextra
  -Wmissing-prototypes
  -Wunreachable-code-break
  -Wmissing-variable-declarations
  -Wfloat-conversion
  -Wunreachable-code-return
  -Wshift-sign-overflow
  -Wkeyword-macro
  -Wno-unused-parameter
  -Wno-sign-compare
  -Wno-shorten-64-to-32
)

# Apply C++ standard specifically to C++ sources
target_compile_features(SDL2 PRIVATE cxx_std_11)

# Link Libraries
target_link_libraries(SDL2 PUBLIC
  GLESv1_CM GLESv2 OpenSLES log android
)

# --- 2b. SDL2 Static Library ---
add_library(SDL2_static STATIC ${SDL_SOURCES})
set_target_properties(SDL2_static PROPERTIES OUTPUT_NAME "SDL2")
target_include_directories(SDL2_static PUBLIC "${REAL_SDL2_SOURCE_DIR}/include")
target_compile_definitions(SDL2_static PUBLIC GL_GLEXT_PROTOTYPES)
target_link_libraries(SDL2_static INTERFACE GLESv1_CM GLESv2 log android)
