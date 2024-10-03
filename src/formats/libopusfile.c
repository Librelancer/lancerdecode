#include "libopusfile.h"

#if (WIN32 || _WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define dynlib_t HMODULE
#define OPEN_LIBRARY LoadLibraryA("libopusfile-0.dll")
#define dlsym GetProcAddress
#else
#include <dlfcn.h>
#define dynlib_t void*
#define OPEN_LIBRARY dlopen("libopusfile.so.0", RTLD_NOW);
#endif

typedef OggOpusFile* (*P_op_open_callbacks)(void*,const OpusFileCallbacks*,const unsigned char*,size_t,int*);
static P_op_open_callbacks _op_open_callbacks;

OggOpusFile *op_open_callbacks(
    void *_source, 
    const OpusFileCallbacks *_cb, 
    const unsigned char *_initial_data, 
    size_t _initial_bytes, 
    int *_error
)
{
    return _op_open_callbacks(_source, _cb, _initial_data, _initial_bytes, _error);
}

typedef void* (*P_op_fdopen)(OpusFileCallbacks*,int,const char*);
static P_op_fdopen _op_fdopen;
void *op_fdopen(OpusFileCallbacks *_cb, int _fd,const char *_mode)
{
    return _op_fdopen(_cb,_fd,_mode);
}

typedef OggOpusFile* (*P_op_open_file)(const char*,int*);
static P_op_open_file _op_open_file;
OggOpusFile *op_open_file(const char *_path,int *_error)
{
    return _op_open_file(_path,_error);
}

typedef ogg_int64_t (*P_op_pcm_tell)(const OggOpusFile*);
static P_op_pcm_tell _op_pcm_tell;
ogg_int64_t op_pcm_tell(const OggOpusFile *_of)
{
    return _op_pcm_tell(_of);
}

typedef opus_int64 (*P_op_raw_tell)(const OggOpusFile*);
static P_op_raw_tell _op_raw_tell;
opus_int64 op_raw_tell(const OggOpusFile *_of)
{
    return _op_raw_tell(_of);
}

typedef ogg_int64_t (*P_op_pcm_total)(const OggOpusFile*,int);
static P_op_pcm_total _op_pcm_total;
ogg_int64_t op_pcm_total(const OggOpusFile *_of,int _li)
{
    return _op_pcm_total(_of, _li);
}

typedef opus_int64 (*P_op_raw_total)(const OggOpusFile*,int);
static P_op_raw_total _op_raw_total;
opus_int64 op_raw_total(const OggOpusFile *_of,int _li)
{
    return _op_raw_total(_of, _li);
}

typedef const OpusHead* (*P_op_head)(const OggOpusFile*,int);
static P_op_head _op_head;
const OpusHead *op_head(const OggOpusFile *_of,int _li)
{
    return _op_head(_of, _li);
}

typedef int (*P_op_seekable)(const OggOpusFile*);
static P_op_seekable _op_seekable;
int op_seekable(const OggOpusFile *_of)
{
    return _op_seekable(_of);
}

typedef int (*P_op_channel_count)(const OggOpusFile*,int);
static P_op_channel_count _op_channel_count;
int op_channel_count(const OggOpusFile *_of, int _li)
{
    return _op_channel_count(_of, _li);
}

typedef int (*P_op_current_link)(const OggOpusFile*);
static P_op_current_link _op_current_link;
int op_current_link(const OggOpusFile *_of)
{
    return _op_current_link(_of);
}

typedef int (*P_op_link_count)(const OggOpusFile*);
static P_op_link_count _op_link_count;
int op_link_count(const OggOpusFile *_of)
{
    return _op_link_count(_of);
}

typedef opus_int32 (*P_op_bitrate_instant)(const OggOpusFile*);
static P_op_bitrate_instant _op_bitrate_instant;
opus_int32 op_bitrate_instant(OggOpusFile *_of)
{
    return _op_bitrate_instant(_of);
}

typedef int (*P_op_read)(OggOpusFile*,int16_t*,int,int*);
static P_op_read _op_read;
int op_read(OggOpusFile *_of, int16_t *pcm, int _buf_size, int *_li)
{
    return _op_read(_of,pcm,_buf_size,_li);
}

typedef int (*P_op_read_stereo)(OggOpusFile*,int16_t*,int);
static P_op_read_stereo _op_read_stereo;
int op_read_stereo(OggOpusFile *_of, int16_t *_pcm, int _buf_size)
{
    return _op_read_stereo(_of, _pcm, _buf_size);
}

typedef int (*P_op_raw_seek)(OggOpusFile*,opus_int64);
static P_op_raw_seek _op_raw_seek;

int op_raw_seek (OggOpusFile *_of, opus_int64 _byte_offset)
{
    return _op_raw_seek(_of, _byte_offset);
}

typedef void (*P_op_free)(OggOpusFile*);
static P_op_free _op_free;
void op_free(OggOpusFile *_of)
{
    _op_free(_of);
}

static int of_opened = 0;
static int of_open_result = 0;

int libopusfile_Open(void)
{
    /* Only attempt to load libopusfile once */
    if(of_opened) {
        return of_open_result;
    }
    of_opened = 1;
    dynlib_t library = OPEN_LIBRARY;
    if(!library) {
        of_open_result = 0;
        return 0;
    }
    /* load symbols */
    _op_open_callbacks = (P_op_open_callbacks)dlsym(library, "op_open_callbacks");
    _op_fdopen = (P_op_fdopen)dlsym(library, "op_fdopen");
    _op_open_file = (P_op_open_file)dlsym(library, "op_open_file");
    _op_pcm_tell = (P_op_pcm_tell)dlsym(library, "op_pcm_tell");
    _op_raw_tell = (P_op_raw_tell)dlsym(library, "op_raw_tell");
    _op_pcm_total = (P_op_pcm_total)dlsym(library, "op_pcm_total");
    _op_raw_total = (P_op_raw_total)dlsym(library, "op_raw_total");
    _op_head = (P_op_head)dlsym(library, "op_head");
    _op_seekable = (P_op_seekable)dlsym(library, "op_seekable");
    _op_channel_count = (P_op_channel_count)dlsym(library, "op_channel_count");
    _op_current_link = (P_op_current_link)dlsym(library, "op_current_link");
    _op_link_count = (P_op_link_count)dlsym(library, "op_link_count");
    _op_bitrate_instant = (P_op_bitrate_instant)dlsym(library, "op_bitrate_instant");
    _op_read = (P_op_read)dlsym(library, "op_read");
    _op_read_stereo = (P_op_read_stereo)dlsym(library, "op_read_stereo");
    _op_free = (P_op_free)dlsym(library, "op_free");
    _op_raw_seek = (P_op_raw_seek)dlsym(library, "op_raw_seek");
    return 1;
}
