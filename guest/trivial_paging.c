/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <trivial_paging.h>
#include <lib.h>
extern uint64_t pml4_base;
//extern uint64_t pdpt_base;
//extern uint64_t pdt_base;

#define PAGE_SIZE_4K 4096
#define PAGE_MASK_4K (PAGE_SIZE_4K - 1)
#define PAGE_SHIFT_4K 12

#define NR_BASE_PAGES 16
__attribute__((aligned(PAGE_SIZE_4K)))
static uint8_t base_pages[NR_BASE_PAGES * PAGE_SIZE_4K];
static int nr_used_base_pages = 0;

static uint64_t
get_base_page(void)
{
    if (nr_used_base_pages >= NR_BASE_PAGES) {
        return 0x0;
    }
    uint8_t *page = &base_pages[nr_used_base_pages];
    nr_used_base_pages++;
    memset(page, 0x0, PAGE_SIZE_4K);
    return (uint64_t)page;
}
// This is to map the vitual page above 4GB
static int
map_page_high(uint64_t va, uint64_t pa)
{
    if (va < 0x100000000) {
        return -1;
    }
    int l4_index = LEVEL_4_INDEX(va);
    int l3_index = LEVEL_3_INDEX(va);
    int l2_index = LEVEL_2_INDEX(va);
    int l1_index = LEVEL_1_INDEX(va);
    struct pml4_entry * l4_entry =
        l4_index + (struct pml4_entry *)&pml4_base;
    if (!l4_entry->present) {
        uint64_t pdp_address = get_base_page();
        if (!pdp_address) {
            return -2;
        }
        memset((void *)pdp_address, 0x0, PAGE_SIZE_4K);
        memset(l4_entry, 0x0, sizeof(struct pml4_entry));
        l4_entry->present = 1;
        l4_entry->writable = 1;
        l4_entry->pdp_entry_address = pdp_address >> PAGE_SHIFT_4K;
    }

    struct pdp_entry * l3_entry = l3_index +
        (struct pdp_entry *)(uint64_t)(l4_entry->pdp_entry_address <<
                                       PAGE_SHIFT_4K);
    if (!l3_entry->present) {
        uint64_t pd_address = get_base_page();
        if (!pd_address) {
            return -3;
        }
        memset((void *)pd_address, 0x0, PAGE_SIZE_4K);
        memset(l3_entry, 0x0, sizeof(struct pdp_entry));
        l3_entry->present = 1;
        l3_entry->writable = 1;
        l3_entry->pd_entry_address = pd_address >> PAGE_SHIFT_4K;
    }

    struct pd_table_entry * l2_table_entry = l2_index +
        (struct pd_table_entry *)(uint64_t)(l3_entry->pd_entry_address <<
                                            PAGE_SHIFT_4K);
    if (!l2_table_entry->present) {
        uint64_t pt_addr = get_base_page();
        if (!pt_addr) {
            return -4;
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
        return -5;
    }
    memset(l1_entry, 0x0, sizeof(struct pt_entry));
    l1_entry->present = 1;
    l1_entry->writable = 1;
    l1_entry->page_address = pa >> PAGE_SHIFT_4K;

    return 0;
}

void
paging_init(void)
{
    // Map video buffer
    map_page_high(0x1000b8000, 0x1000b8000);
    nr_used_base_pages = 0;

    __asm__ volatile("mov %%rax, %%cr3;"
                     :
                     :"a"(&pml4_base)
                     :"memory", "cc");
    print_string("Finishing guest paging.\n");
}
