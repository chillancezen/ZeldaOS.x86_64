/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <device/include/keyboard.h>
#include <x86_64/include/ioapic.h>
#include <lib64/include/logging.h>


void
keyboard_init(void)
{
    ioapic_redirect_entry(1, 0);
    LOG_INFO("Initialize keyboard\n");
}
