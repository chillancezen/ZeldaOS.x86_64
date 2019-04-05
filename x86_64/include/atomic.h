/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _ATOMIC_H
#define _ATOMIC_H
#include <lib64/include/type.h>

static inline uint32_t
xchg32(void * dword_addr, uint32_t exchange_value)
{
    uint32_t old_value = 0;
    __asm__ volatile ("lock;"
                      "xchgl %%eax, %[TARGET_MEM]"
                      :"=a"(old_value)
                      :"a"(exchange_value), [TARGET_MEM]"m"(*(uint32_t *)dword_addr)
                      :"cc");
    return old_value;
}
#endif

