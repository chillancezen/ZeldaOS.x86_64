/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/ioport.h>

inline uint8_t
inb(uint16_t portid)
{
    uint8_t ret;
    asm volatile("inb %1, %0;"
                 :"=a"(ret)
                 :"Nd"(portid));
    return ret;
}

inline void
outb(uint16_t portid, uint8_t val)
{
    asm volatile("outb %%al, %%dx;"
                 :
                 :"a"(val), "Nd"(portid));
}

inline uint16_t
inw(uint16_t portid)
{
    uint16_t ret;
    asm volatile("inw %1, %0;"
                 :"=a"(ret)
                 :"Nd"(portid));
    return ret;
}

inline void
outw(uint16_t portid, uint16_t val)
{
    asm volatile("outw %%ax, %%dx"
                 :
                 :"a"(val), "Nd"(portid));
}

inline uint32_t
inl(uint16_t portid)
{
    uint32_t ret;
    asm volatile("inl %1, %0;"
                 :"=a"(ret)
                 :"Nd"(portid));
    return ret;
}

inline void
outl(uint16_t portid, uint32_t val)
{
    asm volatile("outl %%eax, %%dx;"
                 :
                 :"a"(val), "Nd"(portid));
}

