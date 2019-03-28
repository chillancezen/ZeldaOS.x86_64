/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _IOPORT_H
#define _IOPORT_H
#include <lib64/include/type.h>
uint8_t
inb(uint16_t portid);

void
outb(uint16_t portid, uint8_t val);

uint16_t
inw(uint16_t portid);

void
outw(uint16_t portid, uint16_t val);

uint32_t
inl(uint16_t portid);

void
outl(uint16_t portid, uint32_t val);

#endif
