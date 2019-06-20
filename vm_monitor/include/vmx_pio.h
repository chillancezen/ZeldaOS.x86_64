/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_PIO_H
#define _VMX_PIO_H
#include <vm_monitor/include/vmx_exit.h>

typedef uint8_t inb_callback(uint32_t port_id, struct vmexit_info * exit);
typedef uint16_t inw_callback(uint32_t port_id, struct vmexit_info * exit);
typedef uint32_t inl_callback(uint32_t port_id, struct vmexit_info * exit);
typedef void outb_callback(uint32_t port_id, uint8_t byte, struct vmexit_info * exit);
typedef void outw_callback(uint32_t port_id, uint16_t word, struct vmexit_info * exit);
typedef void outl_callback(uint32_t port_id, uint32_t dword, struct vmexit_info * exit);

struct portio_operation{
    uint32_t portid;
    inb_callback * call_inb;
    inw_callback * call_inw;
    inl_callback * call_inl;
    outb_callback * call_outb;
    outw_callback * call_outw;
    outl_callback * call_outl;
};

#define MAX_PORTS_SUPPORTED 256
#endif
