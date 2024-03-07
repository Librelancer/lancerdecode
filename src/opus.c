#include "formats.h"
#include "libopusfile.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#define OPUS_BUFFER_SIZE 32768

static int libopus_stream_read(void *_stream, unsigned char *_ptr, int _nbytes)
{
    ld_stream_t ld = (ld_stream_t)_stream;
    return (int)ld->read(_ptr, _nbytes, ld);
}

static int libopus_stream_seek(void *_stream, int64_t offset, int whence)
{
    ld_stream_t ld = (ld_stream_t)_stream;
    LDSEEK origin = LDSEEK_SET;
    if(whence == SEEK_CUR) origin = LDSEEK_CUR;
    if(whence == SEEK_END) origin = LDSEEK_END;
    return ld->seek(ld, (int32_t)offset, origin);
}

static int64_t libopus_stream_tell(void *_stream)
{
    ld_stream_t ld = (ld_stream_t)_stream;
    return (int64_t)ld->tell(ld);
}

static int libopus_stream_close(void *_stream)
{
    ld_stream_t ld = (ld_stream_t)_stream;
    ld->close(ld);
    return 0;
}

const char *libopus_strerror(int err)
{
	switch(err) {
        case OP_EREAD:
            return "I/O error";
        case OP_EFAULT:
            return "libopus internal error";
        case OP_EIMPL:
            return "unsupported feature";
        case OP_EINVAL:
            return "invalid value";
        case OP_ENOTFORMAT:
            return "not format";
        case OP_EBADHEADER:
            return "bad header";
        case OP_EVERSION:
            return "version error";
        case OP_EBADLINK:
            return "bad link";
        case OP_EBADTIMESTAMP:
            return "bad timestamp";
        default:
            return "unknown error";
    }
}

typedef struct opus_userdata {
    OggOpusFile *opus;
    int channels;
    int eof;
} opus_userdata_t;

size_t opus_read(void* ptr, size_t size, ld_stream_t stream)
{
    opus_userdata_t *userdata = (opus_userdata_t*)stream->userData;
    if(userdata->eof) return 0;
    size_t sz_bytes = size;
	if((sz_bytes % 2) != 0) {
		LOG_ERROR("opus_read: buffer size must be a multiple of sizeof(short)");
		return 0;
	}
    while(!userdata->eof) {
        int framesRead = 0;
        if(userdata->channels == 2) {
            framesRead = op_read_stereo(userdata->opus, ptr, sz_bytes / 2);
        } else {
            framesRead = op_read(userdata->opus, ptr, sz_bytes / 2, NULL);
        }
        if(framesRead > 0) {
            return framesRead * userdata->channels * 2;
        } else if (framesRead == OP_HOLE) {
            continue;
        } else {
            userdata->eof = 1;
            return 0;
        }
    }
}

int opus_seek(ld_stream_t stream, int32_t offset, LDSEEK origin)
{
	if(origin != LDSEEK_SET || offset != 0) {
		LOG_ERROR("opus_seek: only can seek to LDSEEK_SET 0");
		return -1;
	}
	opus_userdata_t *userdata = (opus_userdata_t*)stream->userData;
    userdata->eof = 0;
    return op_raw_seek(userdata->opus, 0);
}

void opus_close(ld_stream_t stream)
{
	opus_userdata_t *userdata = (opus_userdata_t*)stream->userData;
	op_free(userdata->opus);
	free(userdata);
	free(stream);
}

ld_pcmstream_t opus_getstream(ld_stream_t stream)
{
    if(!libopusfile_Open()) {
        LOG_ERROR("Unable to open libopus");
        return NULL;
    }
    OpusFileCallbacks cb = {
        .read = libopus_stream_read,
        .seek = libopus_stream_seek,
        .tell = libopus_stream_tell,
        .close = libopus_stream_close
    };

    int open_error;
    OggOpusFile *opus = op_open_callbacks(stream, &cb, NULL, 0, &open_error);
    if(!opus) {
        LOG_ERROR_F("opus failed to open: %s", libopus_strerror(open_error));
        stream->close(stream);
        return NULL;
    }

    int channels = op_channel_count(opus, -1);
    if(channels <= 0) {
        op_free(opus);
        LOG_ERROR("opus failed to get channels");
        return NULL;
    }
    if(channels > 2 || op_link_count(opus) != 1) {
        channels = 2;
    }

    opus_userdata_t *userdata = (opus_userdata_t*)malloc(sizeof(opus_userdata_t));
	userdata->channels = channels;
	userdata->eof = 0;
    userdata->opus = opus;
	ld_stream_t data = ld_stream_new();
	data->read = &opus_read;
	data->seek = &opus_seek;
	data->close = &opus_close;
	data->userData = userdata;

    ld_pcmstream_t retsound = (ld_pcmstream_t)malloc(sizeof(struct ld_pcmstream));
	retsound->frequency = 48000;
    retsound->dataSize = -1;
    retsound->blockSize = OPUS_BUFFER_SIZE;
    retsound->stream = data;
    retsound->format = channels == 2 ? LDFORMAT_STEREO16 : LDFORMAT_MONO16;
    return retsound;
}

