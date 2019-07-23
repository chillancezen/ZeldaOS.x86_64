/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <vm_monitor/include/vmx_event.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_exit.h>

void
inject_external_event(struct vmexit_info * exit, int vector)
{
    // The IF bit must be 1 for external interrupt
    ASSERT(vmx_read(GUEST_RFLAG) & (1 << 9));
    uint64_t interrupt_info = 0;
    interrupt_info |= vector & 0xff;
    interrupt_info |= 1 << 31;
    vmx_write(VMENTRY_INTRRUPTION_INFO, interrupt_info);
}


int
is_injectable(struct vmexit_info * exit)
{
    return vmx_read(GUEST_RFLAG) & (1 << 9);
}

uint64_t
interrupt_window_exit_sub_handler(struct vmexit_info * exit)
{
    uint64_t rsp = (uint64_t)exit->vm->vcpu;
    ASSERT(is_injectable(exit));
    // check whether there are pending interrrupts.
    inject_external_event(exit, 32);
    return rsp;
}
