// MIT License - Copyright (c) Callum McGing
// This file is subject to the terms and conditions defined in
// LICENSE, which is part of this source code package

#include "logging.h"
#include <stdarg.h>
#include <stdio.h>

void ld_logf(ld_options_t options, int error, const char *fmt, ...)
{
	ld_msgcallback_t callback = NULL;
	if(options) {
		callback = error 
			? options->msgerror
			: options->msginfo;
	}
    char buffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer,1024,fmt,args);
	if(callback)
        callback(buffer);
	else
        fprintf(error ? stderr : stdout, "%s\n", buffer);
	va_end(args);
}
