// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#ifndef _LANCERDECODE_H_
#define _LANCERDECODE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
typedef int32_t LDFORMAT;
#define LDFORMAT_MONO8 1
#define LDFORMAT_MONO16 2
#define LDFORMAT_STEREO8 3
#define LDFORMAT_STEREO16 4

typedef int32_t LDSEEK;

#define LDSEEK_SET 1
#define LDSEEK_CUR 2
#define LDSEEK_END 3

#define LDEOF -256

#ifdef _WIN32
#define LDEXPORT __declspec(dllexport)
#else
#define LDEXPORT
#endif

typedef void (*ld_msgcallback_t)(const char*);


typedef struct ld_options *ld_options_t;

LDEXPORT ld_options_t ld_options_new();
LDEXPORT void ld_options_set_msginfo(ld_options_t opts, ld_msgcallback_t cb);
LDEXPORT void ld_options_set_msgerror(ld_options_t opts, ld_msgcallback_t cb);
LDEXPORT void ld_options_free(ld_options_t opts);


typedef struct ld_stream *ld_stream_t;
typedef struct ld_pcmstream *ld_pcmstream_t;
typedef struct ld_pcmstream_internal *ld_pcmstream_internal_t;

struct ld_stream {
    size_t (*read)(void* buffer,size_t size,ld_stream_t stream);
    int (*seek)(ld_stream_t stream,int32_t offset,LDSEEK origin);
    int32_t (*tell)(ld_stream_t stream); //Only set for base file streams
    void (*close)(ld_stream_t stream);
    void* userData;
};

/* Allocates an ld_stream_t object (used for FFI)*/
LDEXPORT ld_stream_t ld_stream_new();
/* Frees an ld_stream_t object */
LDEXPORT void ld_stream_destroy(ld_stream_t stream);
/* Opens a file using fopen, and creates an ld_stream_t from it
 * returns NULL on failure */
LDEXPORT ld_stream_t ld_stream_fopen(const char *filename);
/* Creates a child stream beginning at the current position of src and going until
 * src + len. The base stream should not be read from or seeked while this is active as
 * it will make a corrupt state.
 */
LDEXPORT ld_stream_t ld_stream_wrap(ld_stream_t src, int32_t len, int closeparent);
/* fgetc implemented for ld_stream_t */
LDEXPORT int ld_stream_getc(ld_stream_t stream);


struct ld_pcmstream {
	ld_stream_t stream; /* the stream object containing the PCM data */
	int32_t dataSize; /* total PCM size in bytes, or -1 */
	int32_t frequency; /* sample rate e.g. 44100 */
	LDFORMAT format; /* format */
	int32_t blockSize; /* suggested buffer size when reading this audio data */
	ld_pcmstream_internal_t _internal; /* internal use */
};

/* Opens an audio file from stream, initialising a decoder if necessary */
LDEXPORT ld_pcmstream_t ld_pcmstream_open(ld_stream_t stream, ld_options_t options, const char **error);

/* STRING: Codec of the audio file */
#define LD_PROPERTY_CODEC ("ld.codec")
/* STRING: Container of the audio file */
#define LD_PROPERTY_CONTAINER ("ld.container")
/* INTEGER: trim property from Freelancer .wav (already applied to stream) */
#define LD_PROPERTY_FL_TRIM ("fl.trim")
/* INTEGER: max samples in Freelancer .wav (already applied to stream) */
#define LD_PROPERTY_FL_SAMPLES ("fl.samples")
/* INTEGER: trim property from mp3 (applied when not .wav container) */
#define LD_PROPERTY_MP3_TRIM ("mp3.trim")
/* INTEGER: max samples in mp3 (applied when not .wav container) */
#define LD_PROPERTY_MP3_SAMPLES ("mp3.samples")
/* Gets an integer property from an open ld_pcmstream_t */
/* Returns 1 on success */
LDEXPORT int ld_pcmstream_get_int(ld_pcmstream_t stream, const char *property, int* value);
/* Gets a string property from an open ld_pcmstream_t
 * Returns the amount of characters written to buffer
*/
LDEXPORT int ld_pcmstream_get_string(ld_pcmstream_t stream, const char *property, char *buffer, int size);
/* Prints all properties to msginfo/stdout on an open ld_pcmstream_t */
LDEXPORT void ld_pcmstream_print_properties(ld_pcmstream_t stream);
/* Closes the PCM stream */
LDEXPORT void ld_pcmstream_close(ld_pcmstream_t stream);
#ifdef __cplusplus
}
#endif
#endif
