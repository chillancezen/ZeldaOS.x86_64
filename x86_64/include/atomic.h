/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _ATOMIC_H
#define _ATOMIC_H
#include <lib64/include/type.h>

static inline uint32_t
xchg(void * dword_addr, uint32_t exchange_value)
{
    uint32_t new_value = 0;
    __asm__ volatile ("lock;"
                      "xchgl %%eax, %[TARGET_MEM]"
                      :"=a"(new_value)
                      :"a"(exchange_value), [TARGET_MEM]"m"(*(uint32_t *)dword_addr)
                      :"cc");
    return new_value;
}
#endif

