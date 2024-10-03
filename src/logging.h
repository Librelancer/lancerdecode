// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "pcmstream.h"
#include "options.h"

#define LOG_O_ERROR_F(o, x, ...) ld_logf((o), 0, x, __VA_ARGS__);
#define LOG_O_ERROR(o, x) ld_logf((o), 0, x);

#define LOG_O_INFO_F(o, x, ...) ld_logf((o), 1, x, __VA_ARGS__);
#define LOG_O_INFO(o, x, ...) ld_logf((o), 1, x);

#define LOG_S_ERROR_F(o, x, ...) ld_logf(&((o)->_internal->options), 1, x, __VA_ARGS__);
#define LOG_S_ERROR(o, x) ld_logf(&((o)->_internal->options), 1, x);

#define LOG_S_INFO_F(o, x, ...) ld_logf(&((o)->_internal->options), 0, x, __VA_ARGS__);
#define LOG_S_INFO(o, x, ...) ld_logf(&((o)->_internal->options), 0, x);

void ld_logf(ld_options_t options, int error, const char *fmt, ...);
#endif 
