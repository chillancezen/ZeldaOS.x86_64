/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <vm_monitor/include/device_keyboard.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_pio.h>

static uint8_t
keyboard_data_inb(uint32_t port_id, struct vmexit_info * exit)
{
    uint8_t ret = 0;
    struct vmcs_blob * vm = exit->vm;
    struct ring * kbd_ring = vmcs_to_keyboard_buffer(vm);

    ASSERT(port_id == KEYBOARD_DATA_PORT);
    if (!ring_empty(kbd_ring)) {
        ASSERT(1 == ring_dequeue(kbd_ring, &ret));
    }
    return ret;
}

void
vmx_device_keyboard_preinit(void)
{
    struct portio_operation ops;
    memset(&ops, 0x0, sizeof(struct portio_operation));
    ops.portid = KEYBOARD_DATA_PORT;
    ops.call_inb = keyboard_data_inb;
    register_pio_operation(&ops);
}
