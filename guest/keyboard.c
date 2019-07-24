/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <keyboard.h>
#include <lib.h>
#include <interrupt.h>
#include <portio.h>

#define KEYBOARD_DATA_PORT 0x60

uint8_t
try_retrieve_scancode(uint8_t * scancode)
{
    uint8_t code = inb(KEYBOARD_DATA_PORT);
    if (!code) {
        return 0;
    }
    *scancode = code;
    return 1;
}

static uint64_t
keyboard_interrupt_handler(struct cpu_state64 * cpu)
{
    uint8_t code;
    try_retrieve_scancode(&code);    
    printk("keyboard interrupted:%x\n", code);
    return (uint64_t)cpu;
}
void
keyboard_init(void)
{
    register_interrupt_handler(0x21, keyboard_interrupt_handler);
    print_string("finish keyboard initialization\n");
}
