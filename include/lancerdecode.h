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

typedef struct ld_stream *ld_stream_t;
typedef struct ld_pcmstream *ld_pcmstream_t;

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
};

/* Opens an audio file from stream, initialising a decoder if necessary */
LDEXPORT ld_pcmstream_t ld_pcmstream_open(ld_stream_t stream);
/* Closes the PCM stream */
LDEXPORT void ld_pcmstream_close(ld_pcmstream_t stream);

/* Used when lancerdecode wants to print logging information */
typedef void (*ld_errorlog_callback_t)(const char*);
LDEXPORT void ld_errorlog_register(ld_errorlog_callback_t cb);
#ifdef __cplusplus
}
#endif
#endif
