/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _CPU_STATE_H
#define _CPU_STATE_H
#include <lib64/include/type.h>

struct cpu_state64 {
    // general segment registers
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    // general purpose registers
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
    // The vector that caused the interrupt or exception
    uint64_t vector;
    // The error code, optional, but the interrupt service routine entry
    // should always push one even the interrupt or exception does not generate
    // error code
    uint64_t error_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    // in 64-bit mode, the SS and rsp are always pushed into stack
    uint64_t rsp;
    uint64_t ss;
}__attribute__((packed));
#endif
