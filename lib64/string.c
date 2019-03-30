/*
 * Copyright (c) 2019 Jie Zheng
 */


#include <lib64/include/string.h>


void
memset(void * dst, uint8_t val, uint64_t size)
{
    uint64_t idx = 0;
    uint8_t * ptr = (uint8_t *)dst;
    for (; idx < size; idx++) {
        ptr[idx] = val;
    }
}
