/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _GDT_H
#define _GDT_H

#include <lib64/include/type.h>


#define SEG_TYPE_APPLICATION 0x1
#define SEG_TYPE_SYSTEM 0x0

#define SEG_GRANULARITY_BYTE 0x0
#define SEG_GRANULARITY_PAGE 0x1

#define DPL_0 0x0
#define DPL_3 0x3



#define NULL_SELECTOR 0x00
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10
#define USER_CODE_SELECTOR 0x1b
#define USER_DATA_SELECTOR 0x23
#define TSS0_SELECTOR 0x28


struct gdt_entry {
    uint32_t limit_0_15:16;
    uint32_t base_0_15:16;
    uint32_t base_16_23:8;
    uint32_t segment_type:4;
    uint32_t segmet_class:1; //0 = SEG_TYPE_SYSTEM, 1 = SEG_TYPE_APPLICATION
    uint32_t dpl:2;
    uint32_t present:1;
    uint32_t limit_16_19:4;
    uint32_t avail:1;
    uint32_t long_mode:1;
    uint32_t operation_size:1;
    uint32_t granularity:1;
    uint32_t base_24_31:8;
}__attribute__((packed));

struct gdt_tss_entry {
    uint32_t limit_0_15:16;
    uint32_t base_0_15:16;
    uint32_t base_16_23:8;
    uint32_t segment_type:4;
    uint32_t segmet_class:1; // must be 0
    uint32_t dpl:2;
    uint32_t present:1;
    uint32_t limit_16_19:4;
    uint32_t avail:1;
    uint32_t reserved0:2;
    uint32_t granularity:1; // count in byte
    uint32_t base_24_31:8;
    uint32_t base_32_63;
    uint32_t reserved1;
}__attribute__((packed));

struct gdt_info {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed));

void
gdt64_init(void);

void
gdt64_ap_init(void);
#endif
