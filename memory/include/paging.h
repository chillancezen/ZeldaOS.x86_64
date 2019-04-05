/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _PAGING_H
#define _PAGING_H
#include <memory/include/physical_memory.h>

//Level 4 entry
struct pml4_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t pdp_entry_address:52;
}__attribute__((packed));

// Level 3 entry
struct pdp_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t pd_entry_address:52;
}__attribute__((packed));

// level 2 2mb entry
struct pd_2mb_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:2;
    uint64_t page_size:1; // must be 1, or it will reference a page table
    uint64_t reserved1:13;
    uint64_t page_2m_address:43;
}__attribute__((packed));

// level 2 directory entry
struct pd_table_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t pt_entry_address:52;
}__attribute__((packed));

// level 1 page_entry
struct pt_entry {
    uint64_t present:1;
    uint64_t writable:1;
    uint64_t user_accessable:1;
    uint64_t page_writethrough:1;
    uint64_t page_cachedisable:1;
    uint64_t reserved0:7;
    uint64_t page_address:52;
}__attribute__((packed));

#define LEVEL_4_INDEX(addr) ((((uint64_t)(addr)) >> 39) & 0x1ff)
#define LEVEL_3_INDEX(addr) ((((uint64_t)(addr)) >> 30) & 0x1ff)
#define LEVEL_2_INDEX(addr) ((((uint64_t)(addr)) >> 21) & 0x1ff)
#define LEVEL_1_INDEX(addr) ((((uint64_t)(addr)) >> 12) & 0x1ff)


void
paging_init(void);

void
ap_paging_init(void);

int32_t
map_address(uint64_t virt_addr, uint64_t phy_addr, int page_size);

#endif
