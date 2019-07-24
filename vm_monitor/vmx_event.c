/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <vm_monitor/include/vmx_event.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_exit.h>

static uint8_t interrupt_request_bitmap[15];


static void
raise_interrupt_raw(struct vmexit_info * exit, int vector)
{
    ASSERT(vector >= 0 && vector < 15);
    // XXX: not sure whether there is any race condition here.
    // but I do only raise an interrupt in a single CPU.
    // It's sure NO PROBLEM here.
    interrupt_request_bitmap[vector] = 1;
}


static int
is_interrupt_pin_masked(struct vmexit_info * exit, int vector)
{
    ASSERT(vector >= 0 && vector < 15);
    return (vector <= 7 && (exit->vm->pic.master_pic_data & (1 << vector))) ||
           (exit->vm->pic.slave_pic_data & (1 << (vector - 8)));
}

void
raise_interrupt(struct vmexit_info * exit, int vector)
{
    if (is_interrupt_pin_masked(exit, vector)) {
        LOG_TRIVIA("vm:0x%x(vpid:%d) pin:%d masked, interrupt not deliverable\n",
                  exit->vm, exit->vm->vpid, vector);
        return;
    }
    raise_interrupt_raw(exit, vector);
    enable_interrupt_window(exit);
}

static void
inject_external_event(struct vmexit_info * exit, int vector)
{
    // The IF bit must be 1 for external interrupt
    ASSERT(vmx_read(GUEST_RFLAG) & (1 << 9));
    uint64_t interrupt_info = 0;
    interrupt_info |= vector & 0xff;
    interrupt_info |= 1 << 31;
    vmx_write(VMENTRY_INTRRUPTION_INFO, interrupt_info);
}


static int
is_injectable(struct vmexit_info * exit)
{
    return vmx_read(GUEST_RFLAG) & (1 << 9);
}

void
disable_interrupt_window(struct vmexit_info * exit)
{
    vmx_write(CTLS_PRI_PROC_BASED_VM_EXECUTION,
              vmx_read(CTLS_PRI_PROC_BASED_VM_EXECUTION) & ~(1 << 2));
}

void
enable_interrupt_window(struct vmexit_info * exit)
{
    vmx_write(CTLS_PRI_PROC_BASED_VM_EXECUTION,
              vmx_read(CTLS_PRI_PROC_BASED_VM_EXECUTION) | (1 << 2));
}
uint64_t
interrupt_window_exit_sub_handler(struct vmexit_info * exit)
{
    uint64_t rsp = (uint64_t)exit->vm->vcpu;
    // When an interrupt window is open, it's always injectable
    ASSERT(is_injectable(exit));
    // check whether there are pending interrrupts that can be delivered
    if (!exit->vm->pic.interrupt_delivery_pending) {
        // Clean the bit for the vector delivered the very last time
        if (exit->vm->pic.interrupt_delivery_vector != PIC_INVALID_VECTOR) {
            ASSERT(exit->vm->pic.interrupt_delivery_vector >=0 &&
                   exit->vm->pic.interrupt_delivery_vector < 15);
            // XXX: Here needs special attention, we need to keep keyboard
            // interrupt pin alerted if the keyboard buffer is till non-empty
            // it works fine for keyboard emulation. 
            // FIXME: We'd better have a framework to do the post interrupt
            // delivery work in PIT EOI context or interrupt window context 
            if (exit->vm->pic.interrupt_delivery_vector == 1) {
                struct ring * kbd_ring = vmcs_to_keyboard_buffer(exit->vm);
                uint8_t dummy;
                ring_dequeue(kbd_ring, &dummy);
                if (ring_empty(kbd_ring)) {
                    interrupt_request_bitmap[1] = 0;
                    exit->vm->pic.interrupt_delivery_vector = PIC_INVALID_VECTOR;
                }
            } else {
                interrupt_request_bitmap[exit->vm->pic.interrupt_delivery_vector] = 0;
                exit->vm->pic.interrupt_delivery_vector = PIC_INVALID_VECTOR;
            }
        }
        // Find the triggered interrupt. deliver it 
        int idx = 0;
        for (idx = 0; idx < 15; idx++) {
            if (interrupt_request_bitmap[idx]) {
                break;
            }
        }
        if (idx < 15) {
            inject_external_event(exit, exit->vm->pic.master_pic_offset + idx);
            exit->vm->pic.interrupt_delivery_vector = idx;
            exit->vm->pic.interrupt_delivery_pending = 1;
        }
    }
    // Disable interrupt until other events are going to be delivered
    // if there are multiple interrupt requests pending, leave the window open
    // until all the the interrupts have been delivered. 
    if (!exit->vm->pic.interrupt_delivery_pending) {
        disable_interrupt_window(exit);
    }
    return rsp;
}

static void __attribute__((constructor))
vmx_event_preinit(void)
{
    memset(interrupt_request_bitmap, 0x0, sizeof(interrupt_request_bitmap));
}
