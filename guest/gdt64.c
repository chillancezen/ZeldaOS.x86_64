/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <stdint.h>

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
    uint32_t segmet_class:1;
    uint32_t dpl:2;
    uint32_t present:1;
    uint32_t limit_16_19:4;
    uint32_t avail:1;
    uint32_t long_mode:1;
    uint32_t operation_size:1;
    uint32_t granularity:1;
    uint32_t base_24_31:8;
}__attribute__((packed));

struct gdt_info {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed));

#define _SEGMENT_BASE 0x0
#define _SEGMENT_LIMIT -1

#define _SEGMENT_TYPE_RW_DATA 0x2
#define _SEGMENT_TYPE_RX_CODE 0xa
#define _SEGMENT_TYPE_TSS 0x9

__attribute__((aligned(8))) struct gdt_entry initial_gdt_entries[3] = {
    {0},
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RX_CODE, 1,
        DPL_0, 1, _SEGMENT_LIMIT, 0, 1, 0, 1, _SEGMENT_BASE},
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RW_DATA, 1,
        DPL_0, 1, _SEGMENT_LIMIT, 0, 0, 0, 1, _SEGMENT_BASE},
};

__attribute__((aligned(8))) struct gdt_info initial_gdt_info = {
    .offset = (uint64_t)&initial_gdt_entries,
    .size = sizeof(initial_gdt_entries)
};


