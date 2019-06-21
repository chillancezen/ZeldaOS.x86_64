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
    
    __asm__ volatile("movq $0x1000b8000, %%rbx;"
                     "movq %%rax, (%%rbx);"
                     :
                     :
                     :"%rax", "%rbx");
}
