add_library(
  stb
  STATIC
  stb_image_write.c
)

target_include_directories(
  stb
  PUBLIC
  .
)
