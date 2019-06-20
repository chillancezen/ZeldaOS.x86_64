/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_EXIT_H
#define _VMX_EXIT_H
#include <vm_monitor/include/vmx_vmcs.h>

struct vmexit_info {
    struct vmcs_blob * vm;    
    uint64_t basic_reason;
    uint64_t exit_qualification;
    uint64_t guest_linear_addr;
    uint64_t guest_physical_addr;
    uint64_t instruction_length;
    uint64_t instruction_info;
};

// see Table C-1
#define BASIC_VMEXIT_REASON_EXTERNAL_INTERRUPT 1
#define BASIC_VMEXIT_REASON_HLT 12
#define BASIC_VMEXIT_CONTROL_REGISTER_ACCESS 28
#define BASIC_VMEXIT_REASON_IO_INSTRUCTION 30
#define BASIC_VMEXIT_REASON_RDMSR 31
#define BASIC_VMEXIT_REASON_WRMSR 32
#endif
