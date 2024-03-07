// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include "lancerdecode.h"
#include "formats.h"
#include "logging.h"
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


ld_pcmstream_t ogg_getstream(ld_stream_t stream)
{
    oggpage_t page;
    uint8_t segments [256];
    unsigned char ident[9];
    if(stream->read(&page, sizeof(oggpage_t), stream) < sizeof(oggpage_t)) {
        LOG_ERROR("Malformed ogg file: unexpected EOF");
        stream->close(stream);
        return NULL;
    }
    if(stream->read(segments, page.segment_count, stream) < page.segment_count) {
        LOG_ERROR("Malformed ogg file: unexpected EOF");
        stream->close(stream);
        return NULL;
    }
    if(stream->read(ident, 9, stream) < 9) {
        LOG_ERROR("Malformed ogg file: unexpected EOF");
        stream->close(stream);
        return NULL;
    }
    stream->seek(stream,0,LDSEEK_SET);
    if(memcmp(ident, "\x1vorbis", 7) == 0) {
        return vorbis_getstream(stream);
    }
    if(memcmp(ident, "\x7F""FLAC", 5) == 0) {
        return flac_getstream(stream);
    }
    if(memcmp(ident, "OpusHead", 8) == 0) {
        return opus_getstream(stream);
    }
    LOG_ERROR("ogg: unexpected codec or stream found");
    stream->close(stream);
    return NULL;
}

LDEXPORT ld_pcmstream_t ld_pcmstream_open(ld_stream_t stream)
{
	unsigned char magic[4];
	/* Read in magic */
	stream->read(magic,4,stream);
	stream->seek(stream,0,LDSEEK_SET);
	/* Detect file type */
	//Riff
	if(memcmp(magic, "RIFF", 4) == 0) {
		return riff_getstream(stream);
	}
	//Ogg
	if(memcmp(magic, "OggS", 4) == 0) {
		return ogg_getstream(stream);
	}
	//Flac
	if(memcmp(magic, "fLaC", 4) == 0) {
		return flac_getstream(stream);
	}
	//Mp3
	if(memcmp(magic,"ID3", 3) == 0) {
		return mp3_getstream(stream,-1,-1,-1,-1);
	}
	if(magic[0] == 0xFF && magic[1] == 0xFB) {
		return mp3_getstream(stream,-1,-1,-1,-1);
	}

	LOG_ERROR("Unable to detect file type");
	return NULL;
}

LDEXPORT void ld_pcmstream_close(ld_pcmstream_t stream)
{
	stream->stream->close(stream->stream);
	free(stream);
}
