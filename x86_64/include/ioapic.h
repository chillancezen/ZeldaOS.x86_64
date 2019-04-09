/*
 * Copyright (c) 2019 Jie Zheng
 *    see https://wiki.osdev.org/IOAPIC for the detail
 */
#ifndef _IOAPIC_H
#define _IOAPIC_H
#include <lib64/include/type.h>

#define IOAPIC_BASE 0xfec00000

#define IOAPIC_OFFSET_ID 0x0
#define IOAPIC_OFFSET_VERSION 0x1
#define IOAPIC_OFFSET_ARB 0x2
#define IOAPIC_OFFSET_RED_TBL(n) (0x10 + 2 * (n))

#define IOAPIC_REDIRECT_ENTRY_MASKED 0x10000

void
io_apic_init(void);

void
ioapic_redirect_entry(uint32_t entry, uint32_t cpu);
#endif
