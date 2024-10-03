#include "pcmstream.h"
#include "properties.h"
#include "options.h"
#include <stdlib.h>
#include <string.h>

ld_pcmstream_t pcmstream_init(ld_options_t options)
{
    ld_pcmstream_t retsound = (ld_pcmstream_t)malloc(sizeof(struct ld_pcmstream));
    memset(retsound, 0, sizeof(struct ld_pcmstream));
    retsound->_internal = (ld_pcmstream_internal_t)malloc(sizeof(struct ld_pcmstream_internal));
    init_properties(retsound);
    if(options) {
        retsound->_internal->options = *options;
    } else {
        memset(&retsound->_internal->options, 0, sizeof(struct ld_options));
    }
    return retsound;
}

LDEXPORT void ld_pcmstream_close(ld_pcmstream_t stream)
{
	stream->stream->close(stream->stream);
    destroy_properties(stream);
    free(stream->_internal);
	free(stream);
}