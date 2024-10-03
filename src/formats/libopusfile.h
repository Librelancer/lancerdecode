#ifndef _LIBOPUSFILE_H_
#define _LIBOPUSFILE_H_

#include <stdint.h>
#include <stddef.h>

// Function for opening libopusfile
// Returns 1 on success, 0 on failure
// The following functions have undefined behavior when libopusfile is not opened
int libopusfile_Open(void);

typedef int64_t ogg_int64_t;
typedef int64_t opus_int64;
typedef int32_t opus_int32;
typedef uint32_t opus_uint32;
typedef int16_t opus_int16;


#define OP_HOLE          (-3)
#define OP_EREAD        (-128)
#define OP_EFAULT       (-129)
#define OP_EIMPL        (-130)
#define OP_EINVAL       (-131)
#define OP_ENOTFORMAT   (-132)
#define OP_EBADHEADER   (-133)
#define OP_EVERSION     (-134)
#define OP_EBADLINK     (-137)
#define OP_EBADTIMESTAMP (-139)

#define OPUS_CHANNEL_COUNT_MAX (255)

typedef struct OpusHead
{
  int           version;
  int           channel_count;
  unsigned      pre_skip;
  opus_uint32   input_sample_rate;
  int           output_gain;
  int           mapping_family;
  int           stream_count;
  int           coupled_count;
  unsigned char mapping[OPUS_CHANNEL_COUNT_MAX];
} OpusHead;


typedef struct OpusFile_Internal OggOpusFile;

typedef int (*op_read_func)(void *_stream,unsigned char *_ptr,int _nbytes);
typedef int (*op_seek_func)(void *_stream,int64_t _offset,int _whence);
typedef int64_t (*op_tell_func)(void *_stream);
typedef int (*op_close_func)(void *_stream);

typedef struct OpusFileCallbacks {
    op_read_func read;
    op_seek_func seek;
    op_tell_func tell;
    op_close_func close;
} OpusFileCallbacks;

OggOpusFile *op_open_callbacks(
    void *_source, 
    const OpusFileCallbacks *_cb, 
    const unsigned char *_initial_data, 
    size_t _initial_bytes, 
    int *_error
);


void *op_fdopen(OpusFileCallbacks *_cb, int _fd,const char *_mode);

OggOpusFile *op_open_file(const char *_path,int *_error);

ogg_int64_t op_pcm_tell(const OggOpusFile *_of);

opus_int64 op_raw_tell(const OggOpusFile *_of);

ogg_int64_t op_pcm_total(const OggOpusFile *_of,int _li);

opus_int64 op_raw_total(const OggOpusFile *_of,int _li);

const OpusHead *op_head(const OggOpusFile *_of,int _li);

int op_seekable(const OggOpusFile *_of);

int op_channel_count(const OggOpusFile *_of, int _li);

int op_current_link(const OggOpusFile *_of);

int op_link_count(const OggOpusFile *_of);

opus_int32 op_bitrate_instant(OggOpusFile *_of);

int op_read(OggOpusFile *_of, int16_t *pcm, int _buf_size, int *_li);

int op_read_stereo(OggOpusFile *_of, int16_t *_pcm, int _buf_size);

int op_raw_seek (OggOpusFile *_of, opus_int64 _byte_offset);

void op_free(OggOpusFile *_of);
#endif
