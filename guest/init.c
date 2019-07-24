/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <lib.h>
#include <serial.h>
#include <trivial_paging.h>
#include <video.h>
#include <interrupt.h>
#include <pit.h>

void
guest_kernel_main(void)
{
    paging_init();
    interrupt_init();
    pit_init();
    video_init();
}
