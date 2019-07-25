/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <timer.h>
#include <lib.h>
#include <interrupt.h>
#include <tetris.h>

static uint64_t tsc_last;
static uint64_t cpu_freq;
static uint64_t
timer_interrupt_handler(struct cpu_state64 * cpu)
{
    uint64_t tsc_now = rdtsc();
    uint64_t tsc_diff;
    uint64_t msec; 
    if (!tsc_last) {
        tsc_last = tsc_now;
    }
    tsc_diff = tsc_now - tsc_last;
    msec = 1000 * tsc_diff / cpu_freq;
    if (msec >= 500) {
        tsc_last = tsc_now;
        on_key_arrow_down();
    }
    return (uint64_t)cpu;
}

void
timer_init(void)
{
    tsc_last = 0;
    cpu_freq = get_cpu_frequency();
    register_interrupt_handler(0x20, timer_interrupt_handler);
    print_string("finished timer initialization\n");
}
