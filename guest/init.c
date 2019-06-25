/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <lib.h>
#include <serial.h>
#include <trivial_paging.h>
#include <video.h>
#include <interrupt.h>

void
guest_kernel_main(void)
{
    paging_init();
    interrupt_init();
    video_init();
}
