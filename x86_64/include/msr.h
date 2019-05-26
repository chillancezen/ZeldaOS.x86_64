/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _MSR_H
#define _MSR_H

#define APIC_BASE_MSR 0x1b
#define APIC_MSR_BSP_FLAG 0x100
#define APIC_MSR_ENABLE_FLAG 0x800
#define APIC_MSR_X2APIC_FLAG 0x400

#define IA32_EFER_MSR 0xc0000080
#define IA32_EFER_MSR_SCE 0x1
#define IA32_EFER_MSR_LME 0x100
#define IA32_EFER_MSR_LMA 0x400
#define IA32_EFER_MSR_NXE 0x800

#define IA32_FEATURE_CONTROL_MSR 0x3a

#define IA32_VMX_BASIC_MSR 0x480
#define IA32_VMX_CR0_FIXED0_MSR 0x486
#define IA32_VMX_CR0_FIXED1_MSR 0x487
#define IA32_VMX_CR4_FIXED0_MSR 0x488
#define IA32_VMX_CR4_FIXED1_MSR 0x489
#define IA32_VMX_PINBASED_CTLS_MSR 0x481
#define IA32_VMX_PROCBASED_CTLS_MSR 0x482
#define IA32_VMX_PROCBASED_CTLS2_MSR 0x48b
#define IA32_VMX_EPT_VPID_CAP_MSR 0x48c

#define FSBASE_MSR 0xc0000100
#define GSBASE_MSR 0xc0000101
#define KERNEL_GSBASE_MSR 0xc0000102

#if defined(C_CONTEXT)
#include <lib64/include/type.h>

#define RDMSR(msr, eax, edx) { \
    __asm__ volatile("rdmsr;" \
                     :"=a"(*(uint32_t *)(eax)), "=d"(*(uint32_t *)(edx)) \
                     :"c"((uint32_t)(msr))); \
}

#define WRMSR(msr, eax, edx) { \
    __asm__ volatile("wrmsr;" \
                     : \
                     :"a"((uint32_t)(eax)), "d"((uint32_t)(edx)), \
                      "c"((uint32_t)(msr))); \
}

#endif

#endif
