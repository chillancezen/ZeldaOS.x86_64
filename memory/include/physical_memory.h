/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _PHYSICAL_MEMORY_H
#define _PHYSICAL_MEMORY_H
#include <lib64/include/type.h>

#define PAGE_SIZE_4K 4096
#define PAGE_MASK_4K (PAGE_SIZE_4K - 1)

enum memory_segment_type {
    TYPE_NORMAL = 1,
    TYPE_RESERVED = 2
};

struct physical_memory_segment {
    uint64_t address;
    uint64_t length;
    uint32_t type;
    uint32_t reserved0;
}__attribute__((packed));


void
physical_memory_init(void);


uint64_t
get_physical_pages(int nr_pages);

uint64_t
get_physical_page(void);

#endif
