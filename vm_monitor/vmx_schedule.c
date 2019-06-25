/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_schedule.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_event.h>

static uint64_t
external_interrupt_sub_handler(struct external_interrupt_info * interrupt)
{
    switch (interrupt->vector)
    {
        case 0x20: // the LAPIC timer interrupt
            break;
        default:
            PANIC_EXIT(interrupt->exit);
            break;
    }
    return (uint64_t)interrupt->exit->vm->vcpu;
}

uint64_t
external_interrupt_exit_sub_handler(struct vmexit_info * exit)
{
    uint64_t rsp = (uint64_t)exit->vm->vcpu;
    struct external_interrupt_info interrupt;
    uint64_t interrupt_info = vmx_read(VMEXIT_INTERRUPT_INFORMATION);
    ASSERT(((interrupt_info >> 31) & 0x1));
    interrupt.exit = exit;
    interrupt.vector = interrupt_info & 0xff;
    interrupt.interrupt_type = (interrupt_info >> 8) & 0x7;
    interrupt.is_errorcode_set = (interrupt_info >> 11) & 0x1;
    interrupt.error_code = vmx_read(VMEXIT_INTERRUPT_ERROR_CODE);
    switch (interrupt.interrupt_type)
    {
        case 0:// external interrupt
            rsp = external_interrupt_sub_handler(&interrupt);
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    return rsp;
}
