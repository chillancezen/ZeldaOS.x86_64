/*
 * Copyright (c) 2019 Jie Zheng
 *
 *   The CPUID helper functions running in 32-bit mode
 */
#include <x86_64/include/cpuid.h>

.code32
.section .text


// @return in %EAX non-zero if the APIC is supported
.global apic_supported
apic_supported:
    pushal
    movl $0x1, %eax
    cpuid
    andl $CPUID_EDX_FLAG_APIC, %edx
    jz 1f
    popal
    movl $0x1, %eax
    jmp 2f
1:
    popal
    movl $0x0, %eax
2:
    ret


