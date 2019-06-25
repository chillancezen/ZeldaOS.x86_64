/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_SCHEDULE_H
#define _VMX_SCHEDULE_H
#include <vm_monitor/include/vmx_exit.h>
#include <vm_monitor/include/vmx_vmcs.h>

struct external_interrupt_info {
    struct vmexit_info * exit;
    uint16_t vector;
    uint16_t interrupt_type;
    uint16_t is_errorcode_set;
    uint64_t error_code;
};

#endif
