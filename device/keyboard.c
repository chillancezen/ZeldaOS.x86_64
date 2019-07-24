/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <device/include/keyboard.h>
#include <x86_64/include/ioapic.h>
#include <lib64/include/logging.h>
#include <x86_64/include/interrupt.h>
#include <x86_64/include/processor_local_storage.h>
#include <x86_64/include/ioport.h>

#define KEYBOARD_DATA_PORT 0x60

static uint8_t
retrieve_scancode(void)
{
    uint8_t code = 0;
    do {
        code = inb(KEYBOARD_DATA_PORT);
        if (code) {
            break;
        }
    } while (1);
    return code;
}

static uint64_t
keyboard_device_interrupt_handler(struct cpu_state64 * cpu)
{
    uint64_t rsp = (uint64_t)cpu;
    uint32_t scancode = retrieve_scancode();
    LOG_TRIVIA("keyboard interrupt, cpu:%d scancode:0x%x\n", cpuid(), scancode);
    return rsp;
}

void
keyboard_init(void)
{
    register_interrupt_handler(33, keyboard_device_interrupt_handler);
    ioapic_redirect_entry(1, 0);
    LOG_INFO("Initialize keyboard device\n");
}
