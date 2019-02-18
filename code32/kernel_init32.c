/*
 * Copyright (c) 2019 Jie Zheng
 *
 */
#include <code32/include/serial.h>

void
kernel32_init(void)
{
    serial_early_init();
    print_string("Hello Zelda64\n");
}
