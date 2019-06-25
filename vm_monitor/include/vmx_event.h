/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_EVENT_H
#define _VMX_EVENT_H
#include <vm_monitor/include/vmx_exit.h>
#include <vm_monitor/include/vmx_vmcs.h>

void
inject_external_event(struct vmexit_info * exit, int vector);
#endif
