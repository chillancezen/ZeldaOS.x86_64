/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _MISC_H
#define _MISC_H
#include <lib64/include/type.h>

static inline  void
cli(void)
{
    __asm__ volatile("cli;");
}


static inline void
sti(void)
{
    __asm__ volatile("sti;");
}

static inline void
halt(void)
{
    __asm__ volatile("hlt;");
}


static inline void
barrier(void)
{
    __asm__ volatile(""
                     :
                     :
                     :"memory");
}


static inline uint64_t
rdtsc(void)
{
    uint32_t eax = 0;
    uint32_t edx = 0;
    __asm__ volatile("rdtscp;"
                     :"=a"(eax), "=d"(edx)
                     :
                     :"cc");
    return (((uint64_t)edx) << 32)| (uint64_t)eax;
}
#endif
