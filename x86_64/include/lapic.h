/*
 * Copyright (c) 2019 Jie Zheng
 *
 *    Let the CPU work in xAPIC mode, for now, we don't take care of
 *    x2APIC mode
 */
#ifndef _LAPIC_H
#define _LAPIC_H

#define APIC_BASE 0xfee00000


#define APIC_LVT_MASKED 0x10000

#define APIC_ID_REGISTER 0xfee00020
#define APIC_TIMER_REGISTER 0xfee00320
#define APIC_LINT0_REGISTER 0xfee00350
#define APIC_LINT1_REGISTER 0xfee00360
#define APIC_PERF_COUNTER_REGISTER 0xfee00340
#define APIC_ERROR_REGISTER 0xfee00370
#define APIC_ERROR_STATUS_REGISTER 0xfee00280
#define APIC_EOI_REGISTER 0xfee000b0
#define APIC_TASK_PRIORITY_REGISTER 0xfee00080
#define APIC_SPURIOUS_REGISTER 0xfee000f0
#define APIC_SPURIOUS_REGISTER_APIC_ENABLE 0x100
#define APIC_ICR_LOW 0xfee00300
#define APIC_ICR_HIGH 0xfee00310
#define APIC_TDC_REGISTER 0xfee003e0
#define APIC_TIC_REGISTER 0xfee00380
#define APIC_TCC_REGISTER 0xfee00390

#define APIC_TIMER_MODE_PERODIC (0x1 << 17)


#define APIC_ICR_DELIVERY_FIXED 0x000
#define APIC_ICR_DELIVERY_INIT  0x500
#define APIC_ICR_DELIVERY_STARTUP 0x600
#define APIC_ICR_DEST_MODE_PHYSICAL 0x000
#define APIC_ICR_DEST_MODE_LOGICAL 0x800
#define APIC_ICR_DELIVER_STATUS_IDLE 0x0000
#define APIC_ICR_DELIVER_STATUS_SENDPENDING 0x1000
#define APIC_ICR_LEVEL_DEASSERT 0x0000
#define APIC_ICR_LEVEL_ASSERT 0x4000
#define APIC_ICR_TRIGGER_EDGE 0x0000
#define APIC_ICR_TRIGGER_LEVEL 0x8000
#define APIC_ICR_SHORTHAND_NO 0x00000
#define APIC_ICR_SHORTHAND_SELF 0x40000
#define APIC_ICR_SHORTHAND_ALL 0x80000
#define APIC_ICR_SHORTHAND_OTHER 0xc0000

#if defined(C_CONTEXT)
#include <lib64/include/type.h>
// the 0-32 are reserved for intel
// the maskable interrupt is ranging from 32 to 255
#define IRQ_BASE 32
// As a matter of fact, this is the for local APIC timer, not for intel 8253 PIT
#define IRQ_TIMER 0
#define IRQ_KEYBOARD 1
#define IRQ_ERROR 19


void
check_x2apic_mode(void);


void
local_apic_init(void);

void
local_apic_ap_init(void);

int
is_bootstrap_processor(void);

uint32_t
local_apic_base(void);


int
is_local_apic_enabled(void);


void
enable_local_apic(void);


extern uint32_t
cpu(void);

void
acknowledge_interrupt(void);


void
start_other_processors(void);

#endif

#endif
