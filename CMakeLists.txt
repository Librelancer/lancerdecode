cmake_minimum_required (VERSION 3.1)
set(CMAKE_C_STANDARD 99)
project (lancerdecode)

option(BUILD_LANCERDECODE_EXAMPLE "Build the lancerdecode example" FALSE)

add_library(lancerdecode SHARED

src/autoload.c
src/flac.c
src/logging.c
src/mp3.c
src/riff.c
src/stream.c
src/sbuffer.c
src/vorbis.c
src/libopusfile.c
src/opus.c

)

target_include_directories(lancerdecode PUBLIC include)
if (NOT WIN32)
  target_link_libraries(lancerdecode m)
endif()

if(BUILD_LANCERDECODE_EXAMPLE)
  add_executable(lancerdecode_example example.c)
  target_link_libraries(lancerdecode_example lancerdecode)
endif()
