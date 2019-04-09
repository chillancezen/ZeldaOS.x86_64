/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <x86_64/include/lapic.h>
.code32
.section .text

.global enable_apic
enable_apic:
    pushal
    movl $APIC_SPURIOUS_REGISTER, %edi
    movl (%edi), %eax
    orl $0x100, %eax
    movl %eax, (%edi)
    mfence
    popal
    ret

.global broadcast_init_ipi
broadcast_init_ipi:
    pushal
    movl $APIC_ICR_HIGH, %edi
    movl $0x0, (%edi)
    movl $APIC_ICR_LOW, %edi
    xorl %edx, %edx
    orl $APIC_ICR_DELIVERY_INIT, %edx
    orl $APIC_ICR_DEST_MODE_PHYSICAL, %edx
    orl $APIC_ICR_LEVEL_ASSERT, %edx
    orl $APIC_ICR_TRIGGER_EDGE, %edx
    orl $APIC_ICR_SHORTHAND_OTHER, %edx
broadcast_init_ipi_loop_sending:
    movl %edx, (%edi)
    mfence
    movl (%edi), %eax
    andl $APIC_ICR_DELIVER_STATUS_SENDPENDING, %eax
    jnz broadcast_init_ipi_loop_sending
    popal
    ret


.global broadcast_startup_ipi
broadcast_startup_ipi:
    pushal
    movl $APIC_ICR_HIGH, %edi
    movl $0x0, (%edi)
    movl $APIC_ICR_LOW, %edi
    movl $AP_BOOT_BASE, %edx
    shrl $12, %edx
    orl $APIC_ICR_DELIVERY_STARTUP, %edx
    orl $APIC_ICR_DEST_MODE_PHYSICAL, %edx
    orl $APIC_ICR_LEVEL_ASSERT, %edx
    orl $APIC_ICR_TRIGGER_EDGE, %edx
    orl $APIC_ICR_SHORTHAND_OTHER, %edx
    movl %eax, (%edi)
    mfence
    movl (%edi), %eax
    popal
    ret
