/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  Again this is not full functional paging for the kernel. only part of the
 *  core functions are implemented.
 *  Let's first map the 0-2G with 2M pages and part of 3G barrier backed memory
 *  into kernelspace in order to perform some special tasks, for example, LAPIC
 *  configuration. Map them in an exact way.
 */

#include <memory/include/paging.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>

uint64_t pml4_base;

void
paging_init(void)
{
    uint64_t virt_addr;
    pml4_base = get_physical_page();
    memset((void *)pml4_base, 0x0, PAGE_SIZE_4K);
    ASSERT(sizeof(struct pml4_entry) == 8);
    ASSERT(sizeof(struct pdp_entry) == 8);
    ASSERT(sizeof(struct pd_2mb_entry) == 8);
    ASSERT(sizeof(struct pd_table_entry) == 8);
    ASSERT(sizeof(struct pt_entry) == 8);

    for (virt_addr = 0;
         virt_addr < INITIAL_VA_CEILING;
         virt_addr += PAGE_SIZE_2M) {
        ASSERT(map_address(virt_addr, virt_addr, PAGE_SIZE_2M) == ERROR_OK);
    }

    ASSERT(map_address(0xfee00000, 0xfee00000, PAGE_SIZE_4K) == ERROR_OK);
    __asm__ volatile("movq %%rax, %%cr3;"
                     :
                     :"a"(pml4_base));
    LOG_INFO("switch to new PML4 base:0x%x\n", pml4_base);
}


int32_t
map_address(uint64_t virt_addr, uint64_t phy_addr, int page_size)
{
    ASSERT(page_size == PAGE_SIZE_4K || page_size == PAGE_SIZE_2M);
    int l4_index = LEVEL_4_INDEX(virt_addr);
    int l3_index = LEVEL_3_INDEX(virt_addr);
    int l2_index = LEVEL_2_INDEX(virt_addr);
    int l1_index = LEVEL_2_INDEX(virt_addr);

    // Try to install level 4 entry
    struct pml4_entry * l4_entry =
        l4_index + (struct pml4_entry *)pml4_base;
    if (!l4_entry->present) {
        uint64_t pdp_address = get_physical_page();
        if (!pdp_address) {
            LOG_TRIVIA("Not able to allocate a page as pdp entry\n");
            return -ERROR_OUT_OF_MEMORY;
        }
        memset((void *)pdp_address, 0x0, PAGE_SIZE_4K);
        memset(l4_entry, 0x0, sizeof(struct pml4_entry));
        l4_entry->present = 1;
        l4_entry->writable = 1;
        l4_entry->pdp_entry_address = pdp_address >> PAGE_SHIFT_4K;
    }

    //Try to install level 3 entry
    struct pdp_entry * l3_entry = l3_index +
        (struct pdp_entry *)(uint64_t)(l4_entry->pdp_entry_address <<
                                       PAGE_SHIFT_4K);
    if (!l3_entry->present) {
        uint64_t pd_address = get_physical_page();
        if (!pd_address) {
            LOG_TRIVIA("Not able to alocate a page as pd entry\n");
            return -ERROR_OUT_OF_MEMORY;
        }
        memset((void *)pd_address, 0x0, PAGE_SIZE_4K);
        memset(l3_entry, 0x0, sizeof(struct pdp_entry));
        l3_entry->present = 1;
        l3_entry->writable = 1;
        l3_entry->pd_entry_address = pd_address >> PAGE_SHIFT_4K;
    }

    // Try to install level Level 2 entry
    if (page_size == PAGE_SIZE_2M) {
        struct pd_2mb_entry * l2_2mb_entry = l2_index +
            (struct pd_2mb_entry *)(uint64_t)(l3_entry->pd_entry_address <<
                                              PAGE_SHIFT_4K);
        if (l2_2mb_entry->present) {
            return -ERROR_DUPLICATION;
        }
        ASSERT(!(phy_addr & PAGE_MASK_2M))
        memset(l2_2mb_entry, 0x0, sizeof(struct pd_2mb_entry));
        l2_2mb_entry->present = 1;
        l2_2mb_entry->writable = 1;
        l2_2mb_entry->page_size = 1;
        l2_2mb_entry->page_2m_address = phy_addr >> PAGE_SHIFT_2M;
        return ERROR_OK;
    }

    struct pd_table_entry * l2_table_entry = l2_index +
        (struct pd_table_entry *)(uint64_t)(l3_entry->pd_entry_address <<
                                            PAGE_SHIFT_4K);
    if (!l2_table_entry->present) {
        uint64_t pt_addr = get_physical_page();
        if (!pt_addr) {
            LOG_TRIVIA("Not able to allocate a page as pt entry");
            return -ERROR_OUT_OF_MEMORY;
        }
        memset(l2_table_entry, 0x0, sizeof(struct pd_table_entry));
        memset((void *)pt_addr, 0x0, PAGE_SIZE_4K);
        l2_table_entry->present = 1;
        l2_table_entry->writable = 1;
        l2_table_entry->pt_entry_address = pt_addr >> PAGE_SHIFT_4K;
    }

    // Try to install Level 1 entry
    struct pt_entry * l1_entry = l1_index +
        (struct pt_entry *)(uint64_t)(l2_table_entry->pt_entry_address <<
                                      PAGE_SHIFT_4K);
    if (l1_entry->present) {
        return -ERROR_DUPLICATION;
    }
    ASSERT(!(phy_addr & PAGE_MASK_4K));
    memset(l1_entry, 0x0, sizeof(struct pt_entry));
    l1_entry->present = 1;
    l1_entry->writable = 1;
    l1_entry->page_address = phy_addr >> PAGE_SHIFT_4K;
    return ERROR_OK;
}
