// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include "lancerdecode.h"
#include "formats.h"
#include "logging.h"
#include "properties.h"
#include <string.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct oggpage {
    uint32_t capture_pattern;
    uint8_t version;
    uint8_t header_type;
    uint64_t granule;
    uint32_t bitstream_serial;
    uint32_t page_sequence;
    uint32_t checksum;
    uint8_t segment_count;
} oggpage_t;
#pragma pack(pop)


ld_pcmstream_t ogg_getstream(ld_stream_t stream, ld_options_t options, const char **error)
{
    oggpage_t page;
    uint8_t segments [256];
    unsigned char ident[9];
    if(stream->read(&page, sizeof(oggpage_t), stream) < sizeof(oggpage_t)) {
        LOG_O_ERROR(options, "Malformed ogg file: unexpected EOF");
        *error = "Malformed ogg file: unexpected EOF";
        stream->close(stream);
        return NULL;
    }
    if(stream->read(segments, page.segment_count, stream) < page.segment_count) {
        LOG_O_ERROR(options, "Malformed ogg file: unexpected EOF");
        *error = "Malformed ogg file: unexpected EOF";
        stream->close(stream);
        return NULL;
    }
    if(stream->read(ident, 9, stream) < 9) {
        LOG_O_ERROR(options, "Malformed ogg file: unexpected EOF");
        *error = "Malformed ogg file: unexpected EOF";
        stream->close(stream);
        return NULL;
    }
    stream->seek(stream,0,LDSEEK_SET);
    if(memcmp(ident, "\x1vorbis", 7) == 0) {
        return vorbis_getstream(stream, options, error);
    }
    if(memcmp(ident, "\x7F""FLAC", 5) == 0) {
        return flac_getstream(stream, options, error, 1);
    }
    if(memcmp(ident, "OpusHead", 8) == 0) {
        return opus_getstream(stream, options, error);
    }
    LOG_O_ERROR(options, "ogg: unexpected codec or stream found");
    *error = "ogg: unexpected codec or stream found";
    stream->close(stream);
    return NULL;
}

static int drmp3_hdr_valid(unsigned char *h)
{
    #define DRMP3_HDR_GET_LAYER(h)            (((h[1]) >> 1) & 3)
    #define DRMP3_HDR_GET_BITRATE(h)          ((h[2]) >> 4)
    #define DRMP3_HDR_GET_SAMPLE_RATE(h)      (((h[2]) >> 2) & 3)
    return h[0] == 0xff &&
        ((h[1] & 0xF0) == 0xf0 || (h[1] & 0xFE) == 0xe2) &&
        (DRMP3_HDR_GET_LAYER(h) != 0) &&
        (DRMP3_HDR_GET_BITRATE(h) != 15) &&
        (DRMP3_HDR_GET_SAMPLE_RATE(h) != 3);
    
    #undef DRMP3_HDR_GET_LAYER
    #undef DRMP3_HDR_GET_BITRATE
    #undef DRMP3_HDR_GET_SAMPLE_RATE
}

LDEXPORT ld_pcmstream_t ld_pcmstream_open(ld_stream_t stream, ld_options_t options, const char **error)
{
    // Provide valid error string pointer
    const char *errorStack = NULL;
    const char **errorOut = error ? error : &errorStack;

	unsigned char magic[4];
	/* Read in magic */
	stream->read(magic,4,stream);
	stream->seek(stream,0,LDSEEK_SET);
	/* Detect file type */
	//Riff
	if(memcmp(magic, "RIFF", 4) == 0) {
		return riff_getstream(stream, options, errorOut);
	}
	//Ogg
	if(memcmp(magic, "OggS", 4) == 0) {
		return ogg_getstream(stream, options, errorOut);
	}
	//Flac
	if(memcmp(magic, "fLaC", 4) == 0) {
		return flac_getstream(stream, options, errorOut, 0);
	}
	//Mp3
	if(memcmp(magic,"ID3", 3) == 0 || drmp3_hdr_valid(magic)) {
		return mp3_getstream(stream, options, errorOut, -1,-1,-1,-1);
	}

    *errorOut = "Unable to detect file type";
	LOG_O_ERROR(options, "Unable to detect file type");
	return NULL;
}
