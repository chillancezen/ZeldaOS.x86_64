/*
 * Copyright (c) 2019 Jie Zheng
 *
 *    The init routines.
 */
#include <lib64/include/type.h>
#include <kernel/include/kernel.h>

extern void * _kernel64_constructor_start;
extern void * _kernel64_constructor_end;

static void
pre_init(void)
{
    uint64_t start_addr = (uint64_t)&_kernel64_constructor_start;
    uint64_t end_addr = (uint64_t)&_kernel64_constructor_end;
    uint64_t func_container = 0;
    for (func_container = start_addr;
         func_container < end_addr;
         func_container += 8) {
        ((void (*)(void))*(uint64_t *)func_container)();
    }
}
void
kernel_main(void)
{

    pre_init();
    halt();
}
