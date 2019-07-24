/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <portio.h>
#include <pit.h>
#include <lib.h>

#define TIMER_RESOLUTION_HZ 100

static void
refresh_pit_channel0(void)
{
    int divisor = OSCILLATPR_CHIP_FREQUENCY / TIMER_RESOLUTION_HZ;
    outb(PIT_CONTROL_PORT, 0x6 | 0x30);
    outb(PIT_CHANNEL0_PORT, divisor & 0xff);
    outb(PIT_CHANNEL0_PORT, (divisor >> 8) & 0xff);
}


void
pit_init(void)
{
    refresh_pit_channel0();
    print_string("finish pit initlization\n");
}
