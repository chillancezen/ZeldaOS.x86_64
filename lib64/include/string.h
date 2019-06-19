/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _STRING_H
#define _STRING_H

#include <stdarg.h>
#include <lib64/include/type.h>

void
memset(void * dst, uint8_t val, uint64_t size);

void
memcpy(void * dst, const void * src, int length);

void
sprintf_raw(char * buff, const char * fmt, va_list arg_ptr);


void
sprintf(char * buff, const char * fmt, ...);


#endif
