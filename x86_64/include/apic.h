/*
 * Copyright (c) 2019 Jie Zheng
 *
 *    Let the CPU work in xAPIC mode, for now, we don't take care of
 *    x2APIC mode
 */
#ifndef _APIC_H
#define _APIC_H

#define APIC_BASE 0xfee00000
#define APIC_SPURIOUS_REGISTER 0xfee000f0

#define APIC_ICR_LOW 0xfee00300
#define APIC_ICR_HIGH 0xfee00310


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
void
check_x2apic_mode(void);
#endif

#endif
