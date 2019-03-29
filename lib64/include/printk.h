/*
 * Copyright (c) 2019 Jie Zheng
 * 
 *  The text output functions in kernel
 */

#ifndef _PRINTK_H
#define _PRINTK_H

#include <lib64/include/type.h>
//Added one new formatter: %q, for 64-bit integer, output as decimal
void
printk(const char * fmt, ...);

#endif

