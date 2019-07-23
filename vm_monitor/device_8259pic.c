/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/device_8259pic.h>
#include <vm_monitor/include/vmx_pio.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>

// Document:https://wiki.osdev.org/PIC
// XXX: maintain only one instance of PIC registers for debug reason.

static uint8_t master_pic_command = 0;
static uint8_t slave_pic_command = 0;
static uint8_t master_pic_data = 0;
static uint8_t slave_pic_data = 0;

static uint8_t
master_pic_inb(uint32_t port_id, struct vmexit_info * exit)
{

    uint8_t ret = 0;
    switch(port_id)
    {
        case PIC_MASTER_DATA_PORT:
            ret = master_pic_data; 
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
    switch(port_id)
    {
        case PIC_MASTER_COMMAND_PORT:
            master_pic_command = byte;
            break;
        case PIC_MASTER_DATA_PORT:
            master_pic_data = byte;
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
}


static uint8_t
slave_pic_inb(uint32_t port_id, struct vmexit_info * exit)
{
    uint8_t ret = 0;
    switch(port_id)
    {
        case PIC_SLAVE_DATA_PORT:
            ret = slave_pic_data;
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
    switch(port_id)
    {
        case PIC_SLAVE_COMMAND_PORT:
            slave_pic_command = byte;
            break;
        case PIC_SLAVE_DATA_PORT:
            slave_pic_data = byte;
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
