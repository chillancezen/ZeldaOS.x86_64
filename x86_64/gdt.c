/*
 * Copyright (c) 2019 Jie Zheng
 *
 * the 64bit long mde segementation
 */

#include <x86_64/include/gdt.h>
#include <x86_64/include/tss.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>

#define _SEGMENT_BASE 0x0
#define _SEGMENT_LIMIT -1

#define _SEGMENT_TYPE_RW_DATA 0x2
#define _SEGMENT_TYPE_RX_CODE 0xa
#define _SEGMENT_TYPE_TSS 0x9

//XXX: The 22th bit must be zero in 64bit long mode, otherwise it can cause #GP
//with KVM enabled, note the TSS system descriptor takes 16 byte,  twice
//large as the segment descriptor is
__attribute__((aligned(8))) struct gdt_entry gdt_entries[5 + MAX_NR_CPUS * 2] =
{
    //null segment
    {0},
    //kernel code segment running in Long Mode
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RX_CODE, 1,
        DPL_0, 1, _SEGMENT_LIMIT, 0, 1, 0, 1, _SEGMENT_BASE},
    //kernel data segment
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RW_DATA, 1,
        DPL_0, 1, _SEGMENT_LIMIT, 0, 0, 0, 1, _SEGMENT_BASE},
    //user code segment running in Long Mode
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RX_CODE, 1,
        DPL_3, 1, _SEGMENT_LIMIT, 0, 1, 0, 1, _SEGMENT_BASE},
    //user data segment
    {_SEGMENT_LIMIT, _SEGMENT_BASE, _SEGMENT_BASE, _SEGMENT_TYPE_RW_DATA, 1,
        DPL_3, 1, _SEGMENT_LIMIT, 0, 1, 0, 1, _SEGMENT_BASE},
    //per-cpu tast state segments
};
__attribute__((aligned(64))) static struct gdt_info gdt;


static void
load_gdt(struct gdt_info * gdt)
{
    gdt->size = sizeof(gdt_entries) - 1;
    gdt->offset = (uint64_t)gdt_entries;
    __asm__ volatile("lgdt (%%rax);"
                     :
                     :"a"(gdt)
                     :"memory");
    {
        struct gdt_info _gdt;
        __asm__ volatile("sgdt (%%rax);"
                         :
                         :"a"(&_gdt)
                         :"memory");
        ASSERT(_gdt.size == gdt->size);
        ASSERT(_gdt.offset == gdt->offset);
    }
}
static void
initialize_tss_entries(void)
{
    struct task_state_segment64 * tss_base = get_tss_base();
    int idx = 0;
    for (idx = 0; idx < MAX_NR_CPUS; idx++) {
        uint32_t tss_limit = sizeof(struct task_state_segment64) - 1;
        uint64_t tss_addr = (uint64_t)&tss_base[idx];
        struct gdt_tss_entry * _tss_segment_entry =
            (struct gdt_tss_entry *)(idx * sizeof(struct gdt_tss_entry) +
                                     5 * sizeof (struct gdt_entry) +
                                     (uint64_t)gdt_entries);
        memset(_tss_segment_entry, 0x0, sizeof(struct gdt_tss_entry));
        _tss_segment_entry->limit_0_15 = tss_limit;
        _tss_segment_entry->base_0_15 = tss_addr;
        _tss_segment_entry->base_16_23 = tss_addr >> 16;
        _tss_segment_entry->segment_type = 0x9;
        _tss_segment_entry->segmet_class = 0;
        _tss_segment_entry->dpl = DPL_0;//Let's define it as PL0
        _tss_segment_entry->present = 1;
        _tss_segment_entry->limit_16_19 = tss_limit >> 16;
        _tss_segment_entry->avail = 1;
        _tss_segment_entry->granularity = 0;
        _tss_segment_entry->base_24_31 = tss_addr >> 24;
        _tss_segment_entry->base_32_63 = tss_addr >> 32;
    }
}

void
gdt64_init(void)
{
    ASSERT(sizeof(struct gdt_entry) == 8);
    ASSERT(sizeof(struct gdt_tss_entry) == 16)
    initialize_tss_entries();
    load_gdt(&gdt);
    LOG_INFO("load 64-bit gdt\n");
}
