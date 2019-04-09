/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <x86_64/include/interrupt.h>
#include <x86_64/include/lapic.h>
#include <lib64/include/logging.h>
#include <init/include/lapic_timer.h>

static uint64_t
local_apic_timer(struct cpu_state64 *cpu)
{
    uint64_t rsp = (uint64_t)cpu;
    return rsp;
}
void
lapic_timer_init(void)
{
    register_interrupt_handler(IRQ_BASE + IRQ_TIMER, local_apic_timer);
}
