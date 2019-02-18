/*
 * Copyright (c) 2019 Jie Zheng
 *
 * The kernel init functions during boot
 */
#include <device/include/serial.h>

__attribute__((section(".kernel_init64_fn"))) void
kernel_boot_init(void)
{
    serial_early_init();

    while (1) {
        __asm__ volatile("1:cli;"
            "hlt;"
            "jmp 1b;");
    };
}
