/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <vm_monitor/include/vmx_event.h>
#include <lib64/include/logging.h>

void
inject_external_event(struct vmexit_info * exit, int vector)
{
    uint64_t interrupt_info = 0;
    interrupt_info |= vector & 0xff;
    interrupt_info |= 1 << 31;
    vmx_write(VMENTRY_INTRRUPTION_INFO, interrupt_info);
    // The IF bit must be 1 for external interrupt
    vmx_write(GUEST_RFLAG, vmx_read(GUEST_RFLAG) | (1 << 9));
}
