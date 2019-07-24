/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <timer.h>
#include <lib.h>
#include <interrupt.h>

static uint64_t
timer_interrupt_handler(struct cpu_state64 * cpu)
{
    return (uint64_t)cpu;
}

void
timer_init(void)
{
    register_interrupt_handler(0x20, timer_interrupt_handler);
    print_string("finished timer initialization\n");
}
