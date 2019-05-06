/*
 * Copyright (c) 2019 Jie Zheng
 *   we are gonna use native x86_64 ABI, that's to say, we can use the host
 *   native std header
 */

#ifndef _TYPE_H
#define _TYPE_H

#include <stdint.h>
#include <stdarg.h>

enum ERROR_CODE {
    ERROR_OK = 0,
    ERROR_INVALID,
    ERROR_NOT_NECESSARY,
    ERROR_DUPLICATION,
    ERROR_OUT_OF_MEMORY,
    ERROR_OUT_OF_RESOURCE,
};


#define LIKELY(exp) __builtin_expect(!!(exp), 1)
#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)

#endif
