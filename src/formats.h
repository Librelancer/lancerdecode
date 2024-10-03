// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#ifndef _FORMATS_H_
#define _FORMATS_H_
#include "lancerdecode.h"


ld_pcmstream_t riff_getstream(ld_stream_t stream, ld_options_t options, const char **error);
ld_pcmstream_t mp3_getstream(ld_stream_t stream, ld_options_t options, const char **error, int decodeChannels, int decodeRate, int trimFrames, int totalFrames);
ld_pcmstream_t vorbis_getstream(ld_stream_t stream, ld_options_t options, const char **error);
ld_pcmstream_t flac_getstream(ld_stream_t stream, ld_options_t options, const char **error, int isOgg);
ld_pcmstream_t opus_getstream(ld_stream_t stream, ld_options_t options, const char **error);

#endif 
