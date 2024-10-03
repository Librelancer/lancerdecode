// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include "lancerdecode.h"
#include "formats.h"
#include "logging.h"
#include "properties.h"
#include <stdlib.h>

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#define MP3_BUFFER_SIZE 8192
#include "dr_mp3.h"


typedef struct {
	drmp3 dec;
	ld_stream_t baseStream;
	ld_pcmstream_t pcm;
	void *floatBuffer;
	int floatBufferSize;
	int currentFrames;
	int totalFrames;
	int trimFrames;
} mp3_userdata_t;


size_t read_stream_drmp3(void *pUserData, void *pBufferOut, size_t size)
{
	ld_stream_t stream = (ld_stream_t)pUserData;
	return stream->read(pBufferOut, size, stream);
}

drmp3_bool32 seek_stream_drmp3(void *pUserData, int offset, drmp3_seek_origin origin)
{
	ld_stream_t stream = (ld_stream_t)pUserData;
	int or;
	if(origin == drmp3_seek_origin_start) or = LDSEEK_SET;
	if(origin == drmp3_seek_origin_current) or = LDSEEK_CUR;
	if(stream->seek(stream, offset, or) < 0)
		return DRMP3_FALSE;
	else
		return DRMP3_TRUE;
}
size_t mp3_read(void* ptr, size_t size, ld_stream_t stream)
{
	mp3_userdata_t *userdata = (mp3_userdata_t*)stream->userData;
	int sz_bytes = (int)(size);
	if((sz_bytes % 2) != 0) {
		LOG_S_ERROR(userdata->pcm, "mp3_read: buffer size must be a multiple of sizeof(short)");
		return 0;
	}

	int requestedFrames = sz_bytes / (2 * userdata->dec.channels);
	if(userdata->totalFrames != -1 && ((requestedFrames + userdata->currentFrames) > userdata->totalFrames)) {
		requestedFrames = userdata->totalFrames - userdata->currentFrames;
		if(requestedFrames <= 0) {
			return (size_t)0;
		}
	}
	int floatsz = requestedFrames * userdata->dec.channels * sizeof(float);
	if(userdata->floatBufferSize != floatsz) {
		if(userdata->floatBuffer) free(userdata->floatBuffer);
		userdata->floatBuffer = malloc(floatsz);
		userdata->floatBufferSize = floatsz;
	}
	drmp3_uint64 fcount = drmp3_read_f32(&userdata->dec, (drmp3_uint64)requestedFrames, (float*)userdata->floatBuffer);
	drmp3dec_f32_to_s16((float*)userdata->floatBuffer, (drmp3_int16*)ptr, (int)(fcount * userdata->dec.channels));
	userdata->currentFrames += (int)fcount;
	return (size_t)(fcount * userdata->dec.channels * 2);
}
int mp3_seek(ld_stream_t stream, int32_t offset, LDSEEK origin)
{
	mp3_userdata_t *userdata = (mp3_userdata_t*)stream->userData;
	if(origin != LDSEEK_SET || offset != 0) {
		LOG_S_ERROR(userdata->pcm, "mp3: can only seek to SET 0");
		return -1; 
	}
	drmp3_seek_to_frame(&userdata->dec, (drmp3_uint64)userdata->trimFrames);
	userdata->currentFrames = userdata->trimFrames;
	return 0;	
}

void mp3_close(ld_stream_t stream)
{
	mp3_userdata_t *userdata = (mp3_userdata_t*)stream->userData;
	userdata->baseStream->close(userdata->baseStream);
	if(userdata->floatBuffer) free(userdata->floatBuffer);
	free(userdata);
	free(stream);
}

ld_pcmstream_t mp3_getstream(ld_stream_t stream, ld_options_t options, const char **error, int decodeChannels, int decodeRate, int trimFrames, int totalFrames)
{
	mp3_userdata_t *userdata = (mp3_userdata_t*)malloc(sizeof(mp3_userdata_t));
    memset((void*)userdata, 0, sizeof(mp3_userdata_t));
    drmp3_config drconfig;
	if(decodeRate == -1)
		drconfig.outputSampleRate = 0;
	else
		drconfig.outputSampleRate = decodeRate;
	if(decodeChannels == -1)
		drconfig.outputChannels = 0;
	else
		drconfig.outputChannels = decodeChannels;
	if(!drmp3_init(&userdata->dec,read_stream_drmp3,seek_stream_drmp3,(void*)stream,&drconfig)) {
		LOG_O_ERROR(options, "drmp3_init failed!");
		*error = "drmp3_init failed";
		free(userdata);
		return NULL;
	}
	userdata->baseStream = stream;
	userdata->floatBuffer = NULL;
	userdata->floatBufferSize = -1;
	userdata->trimFrames = (trimFrames == -1 ? 0 : trimFrames);
	userdata->totalFrames = totalFrames;

	ld_stream_t decodeStream = ld_stream_new();
	decodeStream->userData = (void*)userdata;
	decodeStream->read = mp3_read;
	decodeStream->seek = mp3_seek;
	decodeStream->close = mp3_close;
	if(trimFrames != -1) {
		drmp3_seek_to_frame(&userdata->dec, (drmp3_uint64)(trimFrames));
		userdata->currentFrames = trimFrames;
		userdata->totalFrames += trimFrames;
	}
	ld_pcmstream_t retsound = pcmstream_init(options);
	userdata->pcm = retsound;
	retsound->dataSize = -1;
	if(userdata->dec.channels == 2) {
		retsound->format = LDFORMAT_STEREO16;
		//RIFF fact chunk needs channels
	} else {
		retsound->format = LDFORMAT_MONO16;
	}
	retsound->frequency = (int32_t)userdata->dec.sampleRate;
	retsound->stream = decodeStream;
	retsound->blockSize = MP3_BUFFER_SIZE;
    set_property_string(retsound, LD_PROPERTY_CONTAINER, decodeChannels == -1 ? "mp3" : "wav");
    set_property_string(retsound, LD_PROPERTY_CODEC, "mp3");
	if(trimFrames != -1 && totalFrames != -1) {
		set_property_int(retsound, LD_PROPERTY_FL_TRIM, trimFrames);
		set_property_int(retsound, LD_PROPERTY_FL_SAMPLES, totalFrames);
	}
	return retsound;
}
