/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_GUEST_CPU_STATE
#define _VMX_GUEST_CPU_STATE
#include <lib64/include/type.h>
// some of the CPU states are kept in vmcs structure, we do not need to keep
// them ourselves, see the detail from:
// https://rayanfam.com/wp-content/uploads/sites/2/2018/08/VMCS.pdf
// -_-, too much appreciated for anyone who draws it.

struct guest_cpu_state {
    uint64_t cr2;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
}__attribute__((packed));

#endif
