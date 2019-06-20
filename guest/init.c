/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <serial.h>

void
guest_kernel_main(void)
{
    write_serial('a');
}
