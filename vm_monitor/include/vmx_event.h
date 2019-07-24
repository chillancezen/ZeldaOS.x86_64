/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_EVENT_H
#define _VMX_EVENT_H
#include <vm_monitor/include/vmx_exit.h>
#include <vm_monitor/include/vmx_vmcs.h>


uint64_t
interrupt_window_exit_sub_handler(struct vmexit_info * exit);

void
enable_interrupt_window(struct vmexit_info * exit);

void
disable_interrupt_window(struct vmexit_info * exit);

void
raise_interrupt(struct vmexit_info * exit, int vector);
#endif
