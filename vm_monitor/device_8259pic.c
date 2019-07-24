/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/device_8259pic.h>
#include <vm_monitor/include/vmx_pio.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>

// Document:https://wiki.osdev.org/PIC

static uint8_t
master_pic_inb(uint32_t port_id, struct vmexit_info * exit)
{

    struct vmcs_blob * vm = exit->vm;
    uint8_t ret = 0;
    switch(port_id)
    {
        case PIC_MASTER_DATA_PORT:
            ret = vm->pic.master_pic_data; 
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }

    return ret;
}

static void
master_pic_outb(uint32_t port_id, uint8_t byte, struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    switch(port_id)
    {
        case PIC_MASTER_COMMAND_PORT:
            vm->pic.master_pic_command = byte;
            if (vm->pic.master_pic_command & ICW1_INIT) {
                vm->pic.is_master_pic_initializing = 1;
            }
            if (vm->pic.master_pic_command == PIC_EOI) {
                vm->pic.interrupt_delivery_pending = 0;
            }
            break;
        case PIC_MASTER_DATA_PORT:
            vm->pic.master_pic_data = byte;
            if (vm->pic.is_master_pic_initializing) {
                vm->pic.master_pic_offset = vm->pic.master_pic_data;
                vm->pic.is_master_pic_initializing = 0;
                LOG_TRIVIA("vm:0x%x(vpid:%d) master pic offset:%d\n",
                           vm, vm->vpid, vm->pic.master_pic_offset);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
}


static uint8_t
slave_pic_inb(uint32_t port_id, struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    uint8_t ret = 0;
    switch(port_id)
    {
        case PIC_SLAVE_DATA_PORT:
            ret = vm->pic.slave_pic_data;
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    return ret;
}

static void
slave_pic_outb(uint32_t port_id, uint8_t byte, struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    switch(port_id)
    {
        case PIC_SLAVE_COMMAND_PORT:
            vm->pic.slave_pic_command = byte;
            if (vm->pic.slave_pic_command & ICW1_INIT) {
                vm->pic.is_slave_pic_initializing = 1;
            }
            if (vm->pic.slave_pic_command == PIC_EOI) {
                vm->pic.interrupt_delivery_pending = 0;
            }
            break;
        case PIC_SLAVE_DATA_PORT:
            vm->pic.slave_pic_data = byte;
            if (vm->pic.is_slave_pic_initializing) {
                vm->pic.slave_pic_offset = vm->pic.slave_pic_data;
                vm->pic.is_slave_pic_initializing = 0;
                LOG_TRIVIA("vm:0x%x(vpid:%d) slave pic offset:%d\n",
                           vm, vm->vpid, vm->pic.slave_pic_offset);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
}

void
vmx_chipset_8259pic_preinit(void)
{
    struct portio_operation ops;
    int idx = 0;
    for (idx = 0; idx < 2; idx++) {
        memset(&ops, 0x0, sizeof(struct portio_operation));
        ops.portid = PIC_MASTER_COMMAND_PORT + idx;
        ops.call_inb = master_pic_inb;
        ops.call_outb = master_pic_outb;
        register_pio_operation(&ops);

        memset(&ops, 0x0, sizeof(struct portio_operation));
        ops.portid = PIC_SLAVE_COMMAND_PORT + idx;
        ops.call_inb = slave_pic_inb;
        ops.call_outb = slave_pic_outb;
        register_pio_operation(&ops);
    }
}
