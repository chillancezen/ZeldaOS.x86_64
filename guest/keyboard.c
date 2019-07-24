/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <keyboard.h>
#include <lib.h>
#include <interrupt.h>

static uint64_t
keyboard_interrupt_handler(struct cpu_state64 * cpu)
{
    print_string("keyboard interrupted\n");
    return (uint64_t)cpu;
}
void
keyboard_init(void)
{
    register_interrupt_handler(0x21, keyboard_interrupt_handler);
    print_string("finish keyboard initialization\n");
}
