/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <keyboard.h>
#include <lib.h>
#include <interrupt.h>
#include <portio.h>
#include <tetris.h>

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
    switch (code)
    {
        case 0x50: // Down
            on_key_arrow_down();
            break;
        case 0x4b: // Left
            on_key_arrow_left();
            break;
        case 0x4d: // Right
            on_key_arrow_right();
            break;
        case 0x39: // BLANK
            on_key_blank();
            break;
        default:
            //printk("unsupported keyboard scancode:%x\n", code);
            break;

    }
    return (uint64_t)cpu;
}
void
keyboard_init(void)
{
    register_interrupt_handler(0x21, keyboard_interrupt_handler);
    print_string("finish keyboard initialization\n");
}
