#ifndef _OPTIONS_H_
#define _OPTIONS_H_
#include <lancerdecode.h>
struct ld_options {
    ld_msgcallback_t msginfo;
    ld_msgcallback_t msgerror;
};
#endif