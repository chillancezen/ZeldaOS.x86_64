/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _LIB_H
#define _LIB_H
#include <stdint.h>
#include <printk.h>
void
memset(void *dst, const void *src, int nr_bytes);

void
print_string(const char * content);
#endif
