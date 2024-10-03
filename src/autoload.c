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

LDEXPORT ld_pcmstream_t ld_pcmstream_open(ld_stream_t stream, ld_options_t options, const char **error)
{
	unsigned char magic[4];
	/* Read in magic */
	stream->read(magic,4,stream);
	stream->seek(stream,0,LDSEEK_SET);
	/* Detect file type */
	//Riff
	if(memcmp(magic, "RIFF", 4) == 0) {
		return riff_getstream(stream, options, error);
	}
	//Ogg
	if(memcmp(magic, "OggS", 4) == 0) {
		return ogg_getstream(stream, options, error);
	}
	//Flac
	if(memcmp(magic, "fLaC", 4) == 0) {
		return flac_getstream(stream, options, error, 0);
	}
	//Mp3
	if(memcmp(magic,"ID3", 3) == 0) {
		return mp3_getstream(stream, options, error, -1,-1,-1,-1);
	}
	if(magic[0] == 0xFF && magic[1] == 0xFB) {
		return mp3_getstream(stream, options, error, -1,-1,-1,-1);
	}

    *error = "Unable to detect file type";
	LOG_O_ERROR(options, "Unable to detect file type");
	return NULL;
}
