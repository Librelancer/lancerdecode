#define __USE_MINGW_ANSI_STDIO
#include <lancerdecode.h>
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "logging.h"
#ifdef _MSC_VER
#define strdup _strdup
#define strcmp_i(a,b) _stricmp((a), (b))
#else
#include <strings.h>
#define strcmp_i(a,b) strcasecmp((a), (b))
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#endif

typedef struct {
    const char *name;
    int isInteger;
    int integer;
    const char *string;
} property_entry;

int property_compare(const void *a, const void *b, void *udata)
{
    const property_entry *pa = a;
    const property_entry *pb = b;
    return strcmp_i(pa->name, pb->name);
}

uint64_t property_hash(const void *item, uint64_t seed0, uint64_t seed1)
{
    const property_entry *p = item;
    return hashmap_sip(p->name, strlen(p->name), seed0, seed1);
}

int init_properties(ld_pcmstream_t pcmstream)
{
    struct hashmap *h = hashmap_new(sizeof(property_entry), 0, 0, 0, property_hash, property_compare, NULL, NULL);
    pcmstream->_internal->properties = h;
}

void destroy_properties(ld_pcmstream_t pcmstream)
{
    if(pcmstream->_internal->properties) {
        hashmap_free((struct hashmap*)pcmstream->_internal->properties);
        pcmstream->_internal->properties = NULL;
    }
}

void set_property_int(ld_pcmstream_t pcmstream, const char *property, int value)
{
    hashmap_set(pcmstream->_internal->properties, &(property_entry){
        .name = property,
        .isInteger = 1,
        .integer = value,
        .string = NULL
    });
}

void set_property_string(ld_pcmstream_t pcmstream, const char *property, const char *value)
{
    hashmap_set(pcmstream->_internal->properties, &(property_entry){
        .name = property,
        .isInteger = 0,
        .integer = 0,
        .string = value
    });
}


LDEXPORT int ld_pcmstream_get_int(ld_pcmstream_t stream, const char *property, int* value)
{
    const property_entry *result = hashmap_get(stream->_internal->properties, &(property_entry){ .name = property });
    if(result && result->isInteger) {
        if(value) {
            *value = result->integer;
        } 
        return 1;
    }
    return 0;
}

LDEXPORT int ld_pcmstream_get_string(ld_pcmstream_t stream, const char *property, char *buffer, int size)
{
    const property_entry *result = hashmap_get(stream->_internal->properties, &(property_entry){ .name = property });
    if(result) {
        if(result->isInteger) {
            return snprintf(buffer, size, "%d", result->integer);
        } else {
            return snprintf(buffer, size, "%s", result->string);
        }
    }
    return 0;
}

LDEXPORT void ld_pcmstream_print_properties(ld_pcmstream_t stream)
{
    size_t iter = 0;
    void *iter_val;
    while (hashmap_iter (stream->_internal->properties, &iter, &iter_val)) {
        property_entry *pa = iter_val;
        if(pa->isInteger) {
            LOG_S_INFO_F(stream, "%s: %d", pa->name, pa->integer);
        } else {
            LOG_S_INFO_F(stream, "%s: %s", pa->name, pa->string);
        }
    }
}