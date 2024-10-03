#ifndef PCMSTREAM_H_
#define PCMSTREAM_H_
#include "options.h"
struct ld_pcmstream_internal {
    struct ld_options options;
    void *properties;
};
ld_pcmstream_t pcmstream_init(ld_options_t options);
#endif