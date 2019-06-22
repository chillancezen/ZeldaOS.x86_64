/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <lib.h>
#include <serial.h>
#include <trivial_paging.h>

void
guest_kernel_main(void)
{
    paging_init();
    __asm__ volatile("movq $0x12ff230, (%%rbx);"
                     :
                     :"b"(0x1000b8000)
                     :"memory");
#if 0
    uint8_t val = *(uint64_t *)0x1000b8000;
    val += 1;
    *(uint64_t *)0x1000b8000 = 0x2345; 
    __asm__ volatile("movq $0x1000b8000, %%rbx;"
                     "LOCK;""movb %%cl, 0x4(%%rbx);"
                     :
                     :
                     :"%rax", "%rbx");
#endif
}
