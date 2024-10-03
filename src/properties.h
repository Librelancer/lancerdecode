#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_
#include <lancerdecode.h>
int init_properties(ld_pcmstream_t pcmstream);
void set_property_int(ld_pcmstream_t pcmstream, const char *property, int value);
void set_property_string(ld_pcmstream_t pcmstream, const char *property, const char *value);
void destroy_properties(ld_pcmstream_t pcmstream);
#endif