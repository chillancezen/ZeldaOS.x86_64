/*
 * Copyright (c) 2019 Jie Zheng
 *
 * the 64bit long mde segementation
 */

#include <x86_64/include/gdt.h>


#define _SEGMENT_BASE 0x0
#define _SEGMENT_LIMIT -1

#define _SEGMENT_TYPE_RW_DATA 0x2
#define _SEGMENT_TYPE_RX_CODE 0xa
#define _SEGMENT_TYPE_TSS 0x9

//XXX: The 22th bit must be zero in 64bit long mode, otherwise it can cause #GP
//with KVM enabled
__attribute__((aligned(8))) struct gdt_entry gdt_entries[] = {
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
    //static tss0
    {0}
};

void
gdt64_init(void)
{
}
