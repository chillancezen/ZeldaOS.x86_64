/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _PAGING_H
#define _PAGING_H
#include <memory/include/physical_memory.h>

struct pml4_entry {
    uint64_t present:1;
    uint64_t writeable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t pdp_entry_address:52;
}__attribute__((packed));


struct pdp_entry {
    uint64_t present:1;
    uint64_t writeable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t pd_entry_address:52;
}__attribute__((packed));

void
paging_init(void);

#endif
