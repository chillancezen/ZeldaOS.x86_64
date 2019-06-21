/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <memory/include/paging.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_pio.h>
#include <lib64/include/string.h>

#define COM1_PORT_BASE 0x3f8

static uint8_t
device_serial_inb(uint32_t port_id, struct vmexit_info * exit)
{
    uint8_t ret;
    switch(port_id)
    {
        case COM1_PORT_BASE + 5:
            ret = 0x20; // transmission is always ready
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    return ret;
}

static void
device_serial_outb(uint32_t port_id, uint8_t byte, struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    ASSERT(vm->serial_line_iptr < (PAGE_SIZE_4K - 1));
    vm->serial_line_buffer[vm->serial_line_iptr] = byte;
    vm->serial_line_iptr += 1;
    if (vm->serial_line_iptr == (PAGE_SIZE_4K - 1) ||
        byte == '\n') {
        printk("[vm:0x%x(vpid:%d)] %s", vm, vm->vpid, vm->serial_line_buffer);
        memset(vm->serial_line_buffer, 0x0, PAGE_SIZE_4K);
        vm->serial_line_iptr = 0;
    }
}

void
vmx_device_serial_preinit(void)
{
    struct portio_operation ops;
    int idx = 0;
    for (; idx < 6; idx++) {
        memset(&ops, 0x0, sizeof(struct portio_operation));
        ops.portid = COM1_PORT_BASE + idx;
        ops.call_inb = device_serial_inb;
        ops.call_outb = device_serial_outb;
        register_pio_operation(&ops);
    }
}
