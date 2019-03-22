/*
 * Copyright (c) 2019 Jie Zheng
 *
 */

#include <x86_64/include/msr.h>
.code32
.section .text

// @param None
// @return %EAX non-zero if the current processor is BSP
.global check_bootstrap_processor
check_bootstrap_processor:
    pushal
    movl $APIC_BASE_MSR, %ecx
    rdmsr
    andl $APIC_MSR_BSP_FLAG, %eax
    jz 1f
    popal
    movl $0x1, %eax
    jmp 2f
1:
    popal
    movl $0x0, %eax
2:
    ret



