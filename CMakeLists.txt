cmake_minimum_required (VERSION 3.1)
set(CMAKE_C_STANDARD 99)
project (lancerdecode)

add_library(lancerdecode SHARED

src/autoload.c
src/flac.c
src/logging.c
src/mp3.c
src/riff.c
src/stream.c
src/sbuffer.c
src/vorbis.c

)

target_include_directories(lancerdecode PUBLIC include)
