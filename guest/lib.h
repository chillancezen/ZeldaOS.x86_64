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


static inline uint64_t
rdtsc(void)
{
    uint32_t eax = 0;
    uint32_t edx = 0;
    __asm__ volatile("rdtsc;"
                     :"=a"(eax), "=d"(edx)
                     :
                     :"cc");
    return (((uint64_t)edx) << 32)| (uint64_t)eax;
}

static inline int
rand(int range)
{
    return rdtsc() % range;
}


static inline uint64_t
get_cpu_frequency(void)
{
    uint64_t rcx;
    // It's a little tricky here, I put the cpu frequency into RCX after
    // rdtsc is issued. see vm_monitor/vmx_rdtsc.c
    __asm__ volatile("rdtsc;"
                     :"=c"(rcx)
                     :
                     :"memory");
    return rcx;
}
#endif
