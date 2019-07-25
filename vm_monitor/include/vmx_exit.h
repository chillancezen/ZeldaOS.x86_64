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
#define BASIC_VMEXIT_REASON_INTERRUPT_WINDOW 7
#define BASIC_VMEXIT_REASON_HLT 12
#define BASIC_VMEXIT_REASON_RDTSC 16
#define BASIC_VMEXIT_CONTROL_REGISTER_ACCESS 28
#define BASIC_VMEXIT_REASON_IO_INSTRUCTION 30
#define BASIC_VMEXIT_REASON_RDMSR 31
#define BASIC_VMEXIT_REASON_WRMSR 32
#define BASIC_VMEXIT_REASON_EPT_MISCONFIG 49

typedef uint64_t vmexit_sub_handler(struct vmexit_info * exit);

uint64_t
io_instruction_exit_sub_handler(struct vmexit_info * exit);

uint64_t
ept_misconfig_exit_sub_handler(struct vmexit_info * exit);

uint64_t
external_interrupt_exit_sub_handler(struct vmexit_info * exit);

#define GOTO_NEXT_INSTRUCTION(exit) {                                          \
    vmx_write(GUEST_RIP, vmx_read(GUEST_RIP) + (exit)->instruction_length);    \
}

#define PANIC_EXIT(exit) {                                                     \
    dump_vm((exit)->vm);                                                       \
    __not_reach();                                                             \
}

#endif
