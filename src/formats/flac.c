// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include <lancerdecode.h>
#include "../formats.h"
#include "../logging.h"
#include "../properties.h"

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#include "dr_flac.h"

size_t read_stream_drflac(void *pUserData, void *pBufferOut, size_t size)
{
	ld_stream_t stream = (ld_stream_t)pUserData;
	return stream->read(pBufferOut, size, stream);
}

unsigned int seek_stream_drflac(void *pUserData, int offset, drflac_seek_origin origin)
{
	ld_stream_t stream = (ld_stream_t)pUserData;
	int or;
	if(origin == drflac_seek_origin_start) or = LDSEEK_SET;
	if(origin == drflac_seek_origin_current) or = LDSEEK_CUR;
	return !stream->seek(stream, offset, or);
}


typedef struct {
	drflac *pFlac;
	ld_stream_t baseStream;
	ld_pcmstream_t pcm;
} flac_userdata_t;

size_t flac_read(void* ptr, size_t size, ld_stream_t stream)
{
	flac_userdata_t *userdata = (flac_userdata_t*)stream->userData;
	size_t sampleCount = size / 2;
	size_t samplesRead = (size_t)drflac_read_s16(userdata->pFlac, (size_t)sampleCount, (drflac_int16*)ptr);
	return samplesRead * 2;
}

int flac_seek(ld_stream_t stream, int32_t offset, int origin)
{
	flac_userdata_t *userdata = (flac_userdata_t*)stream->userData;
	if(offset != 0 || origin != LDSEEK_SET) {
		LOG_S_ERROR(userdata->pcm, "flac seek only supports reset");
		return 0;
	}
	return drflac_seek_to_sample(userdata->pFlac, 0);
}

void flac_close(ld_stream_t stream)
{
	flac_userdata_t *userdata = (flac_userdata_t*)stream->userData;
	drflac_close(userdata->pFlac);
	userdata->baseStream->close(userdata->baseStream);
	free(userdata);
	free(stream);
}

ld_pcmstream_t flac_getstream(ld_stream_t stream, ld_options_t options, const char **error, int isOgg)
{
	drflac *pFlac = drflac_open(read_stream_drflac, seek_stream_drflac, (void*)stream);
	if(!pFlac) {
		LOG_O_ERROR(options, "Flac decode failed");
		*error = "Flac decode failed";
		stream->close(stream);
		return NULL;
	}

	flac_userdata_t *userdata = (flac_userdata_t*)malloc(sizeof(flac_userdata_t));
	userdata->pFlac = pFlac;
	userdata->baseStream = stream;


	ld_stream_t data = ld_stream_new();
	data->read = &flac_read;
	data->seek = &flac_seek;
	data->close = &flac_close;
	data->userData = userdata;

	ld_pcmstream_t retsound = pcmstream_init(options);
	userdata->pcm = retsound;
	retsound->frequency = pFlac->sampleRate;
	retsound->stream = data;
	retsound->dataSize = -1;
	retsound->blockSize = 8192;
    set_property_string(retsound, LD_PROPERTY_CONTAINER, isOgg ? "ogg" : "flac");
    set_property_string(retsound, LD_PROPERTY_CODEC, "flac");
	if(pFlac->channels == 2) {
		retsound->format = LDFORMAT_STEREO16;
	} else {
		retsound->format = LDFORMAT_MONO16;
	}
	return retsound;
}

