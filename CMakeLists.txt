cmake_minimum_required (VERSION 3.3...3.28)
set(CMAKE_C_STANDARD 99)
project (lancerdecode)

option(BUILD_LANCERDECODE_EXAMPLE "Build the lancerdecode example" FALSE)

add_library(lancerdecode SHARED

src/autoload.c
src/logging.c
src/stream.c
src/sbuffer.c
src/options.c
src/hashmap.c
src/properties.c
src/pcmstream.c

src/formats/flac.c
src/formats/mp3.c
src/formats/riff.c
src/formats/vorbis.c
src/formats/libopusfile.c
src/formats/opus.c
)

target_include_directories(lancerdecode PUBLIC include)
set_target_properties(lancerdecode PROPERTIES C_VISIBILITY_PRESET hidden)
target_compile_definitions(lancerdecode PRIVATE -DBUILDING_LANCERDECODE)

if (NOT WIN32)
  target_link_libraries(lancerdecode m)
endif()

if(BUILD_LANCERDECODE_EXAMPLE)
  add_executable(lancerdecode_example example.c)
  target_link_libraries(lancerdecode_example lancerdecode)
endif()
