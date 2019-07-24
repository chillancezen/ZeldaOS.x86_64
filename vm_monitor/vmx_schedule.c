/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_schedule.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_event.h>
#include <x86_64/include/lapic.h>
#include <x86_64/include/misc.h>
#include <device/include/keyboard.h>

// XXX: When external interrupt occurs, the service routines defined by
// non-vmx code will also be called, in my code, the interrupt is intercepted
// and handled exclusively: this is OK because all the job my kernel does is to
// serve the vm monitor.
static uint64_t
external_interrupt_sub_handler(struct external_interrupt_info * interrupt)
{
    struct vmcs_blob * vm = interrupt->exit->vm;
    switch (interrupt->vector)
    {
        case 0x20: // the LAPIC timer interrupt
            {
                uint64_t curr_tsc = rdtsc();
                if (!vm->pit.last_tsc) {
                    vm->pit.last_tsc = curr_tsc;
                } else {
                    uint64_t diff_mseconds =
                        ((curr_tsc - vm->pit.last_tsc) * 1000) / CPU_FREQUENCY;
                    if (diff_mseconds >= vm->pit.threshold) {
                        // raise an interrupt to the guest
                        raise_interrupt(interrupt->exit, 0x0);
                    }
                    vm->pit.last_tsc = curr_tsc;
                }
            }
            break;
        case 0x21: // The keyboad interrupt
            {
                uint8_t scancode;
                struct ring * kbd_ring = vmcs_to_keyboard_buffer(vm);
                try_retrieve_scancode(&scancode);
                // If the buffer is full and scancode drops, it drops.
                ring_enqueue(kbd_ring, scancode);
                raise_interrupt(interrupt->exit, 0x1);
            }
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
    acknowledge_interrupt();
    return rsp;
}
