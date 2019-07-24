/*
 * Copyright (c) 2019 Jie Zheng
 *
 */

#ifndef _PRINTK_H
#define _PRINTK_H
#include <stdarg.h>
#include <stdint.h>
void
printk(const char *fmt, ...);

void
printk_mp_raw(const char * fmt, va_list arg_ptr);
#endif
