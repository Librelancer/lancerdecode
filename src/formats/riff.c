// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include <lancerdecode.h>
#include "../formats.h"
#include "../logging.h"
#include "../properties.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char chunkID[4];
	uint32_t chunkSize;
	char format[4];
} riff_header_t;

typedef struct {
	char subChunkID[4];
	uint32_t subChunkSize;
	uint16_t audioFormat;
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
} wave_format_t;

typedef struct {
  char subChunkID[4];
  uint32_t subChunk2Size;
} wave_data_t;

#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_MP3 0x55
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#define WAVE_FORMAT_IEEE_FLOAT		0x0003 /* IEEE Float */
#define WAVE_FORMAT_ALAW		0x0006 /* ALAW */
#define WAVE_FORMAT_MULAW		0x0007 /* MULAW */
#define WAVE_FORMAT_IMA_ADPCM		0x0011 /* IMA ADPCM */

ld_pcmstream_t riff_getstream(ld_stream_t stream, ld_options_t options, const char **error)
{
	wave_format_t wave_format;
	riff_header_t riff_header;
	wave_data_t wave_data;
	ld_pcmstream_t retsound;

	stream->read(&riff_header, sizeof(riff_header_t), stream);

	if(memcmp(riff_header.chunkID, "RIFF", 4) != 0 ||
		memcmp(riff_header.format, "WAVE", 4) != 0) {
		LOG_O_ERROR(options, "Invalid RIFF or WAVE header");
		*error = "Invalid RIFF or WAVE header";
		stream->close(stream);
		return 0;
	} 

	stream->read (&wave_format, sizeof(wave_format_t), stream);

	if(memcmp(wave_format.subChunkID, "fmt ", 4) != 0) {
		char actual[5] = { wave_format.subChunkID[0],
		wave_format.subChunkID[1], wave_format.subChunkID[2], wave_format.subChunkID[3], '\0' };
		LOG_O_ERROR_F(options, "Invalid Wave Format :'%s'", actual);
		*error = "Invalid Wave Format";
		stream->close(stream);
		return 0;
	}

	if(wave_format.subChunkSize > 16)
		stream->seek(stream, wave_format.subChunkSize - 16, LDSEEK_CUR);

	int has_data = 0;
	int32_t total_frames = -1;
	int32_t trim_frames = -1;
	while(!has_data) {
		if(!stream->read(&wave_data, sizeof(wave_data_t), stream))
		{
			stream->close(stream);
			LOG_O_ERROR(options, "Unable to find WAVE data");
			*error = "Unable to find WAVE data";
			return 0;
		}
		if(memcmp(wave_data.subChunkID, "data", 4) == 0) {
			has_data = 1;
		} else if (memcmp(wave_data.subChunkID, "fact", 4) == 0) {
			//MP3: Total PCM Samples encoded, trims padding
			stream->read(&total_frames, sizeof(int32_t), stream);
			if(wave_data.subChunk2Size - sizeof(int32_t) > 0)
				stream->seek(stream, wave_data.subChunk2Size - sizeof(int32_t), LDSEEK_CUR);
		} else if (memcmp(wave_data.subChunkID, "trim", 4) == 0) {
			//Freelancer MP3: trim samples at start
			stream->read(&trim_frames, sizeof(int32_t), stream); 
			if(wave_data.subChunk2Size - sizeof(int32_t) > 0)
				stream->seek(stream,wave_data.subChunk2Size - sizeof(int32_t), LDSEEK_CUR);
		} else {
			//skip chunk
			stream->seek(stream, wave_data.subChunk2Size, LDSEEK_CUR);
		}
	}
	if(trim_frames == -1)
		total_frames = trim_frames = -1; //Incomplete data, don't bother trimming
    /*if(total_frames != -1 && (wave_data.subChunk2Size * 8) / total_frames / wave_format.numChannels > 1) {
        //this fact chunk is incorrect, throw away the data
        total_frames = -1;
    }*/
	switch (wave_format.audioFormat) {
		case WAVE_FORMAT_PCM:
			break; //Default decoder
		case WAVE_FORMAT_MP3:
			return mp3_getstream(ld_stream_wrap(stream, wave_data.subChunk2Size, 1), options, error, wave_format.numChannels, wave_format.sampleRate, trim_frames, total_frames);
		default:
			LOG_O_ERROR_F(options, "Unsupported format in WAVE file: '%x'", wave_format.audioFormat);
			*error = "Unsupported format in WAVE file";
			stream->close(stream);
			return 0;
	}

	retsound = pcmstream_init(options);

	if(wave_format.numChannels == 1) {
		if (wave_format.bitsPerSample == 8) {
			retsound->format = LDFORMAT_MONO8;
		} else if (wave_format.bitsPerSample == 16) {
			retsound->format = LDFORMAT_MONO16;
		}
	} else if (wave_format.numChannels == 2) {
		if (wave_format.bitsPerSample == 8) {
			retsound->format = LDFORMAT_STEREO8;
		} else if (wave_format.bitsPerSample == 16) {
			retsound->format = LDFORMAT_STEREO16;
		}
	}

	

	retsound->frequency = wave_format.sampleRate;
	retsound->stream = ld_stream_wrap(stream, wave_data.subChunk2Size, 1);
	retsound->dataSize = wave_data.subChunk2Size;
	retsound->blockSize = 32768;
	init_properties(retsound);
    set_property_string(retsound, LD_PROPERTY_CONTAINER, "wav");
    set_property_string(retsound, LD_PROPERTY_CODEC, "pcm");
	return retsound;
}
