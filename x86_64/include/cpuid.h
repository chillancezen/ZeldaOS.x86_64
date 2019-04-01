/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  The header file contains interfaces to manipulate CPUID instruction
 */


#ifndef _CPUID_H
#define _CPUID_H


#define CPUID_FEAT_ECX_SSE3         (1 << 0)
#define CPUID_FEAT_ECX_PCLMUL       (1 << 1)
#define CPUID_FEAT_ECX_DTES64       (1 << 2)
#define CPUID_FEAT_ECX_MONITOR      (1 << 3)
#define CPUID_FEAT_ECX_DS_CPL       (1 << 4)
#define CPUID_FEAT_ECX_VMX          (1 << 5)
#define CPUID_FEAT_ECX_SMX          (1 << 6)
#define CPUID_FEAT_ECX_EST          (1 << 7)
#define CPUID_FEAT_ECX_TM2          (1 << 8)
#define CPUID_FEAT_ECX_SSSE3        (1 << 9)
#define CPUID_FEAT_ECX_CID          (1 << 10)
#define CPUID_FEAT_ECX_FMA          (1 << 12)
#define CPUID_FEAT_ECX_CX16         (1 << 13)
#define CPUID_FEAT_ECX_ETPRD        (1 << 14)
#define CPUID_FEAT_ECX_PDCM         (1 << 15)
#define CPUID_FEAT_ECX_PCIDE        (1 << 17)
#define CPUID_FEAT_ECX_DCA          (1 << 18)
#define CPUID_FEAT_ECX_SSE4_1       (1 << 19)
#define CPUID_FEAT_ECX_SSE4_2       (1 << 20)
#define CPUID_FEAT_ECX_x2APIC       (1 << 21)
#define CPUID_FEAT_ECX_MOVBE        (1 << 22)
#define CPUID_FEAT_ECX_POPCNT       (1 << 23)
#define CPUID_FEAT_ECX_AES          (1 << 25)
#define CPUID_FEAT_ECX_XSAVE        (1 << 26)
#define CPUID_FEAT_ECX_OSXSAVE      (1 << 27)
#define CPUID_FEAT_ECX_AVX          (1 << 28)
#define CPUID_FEAT_EDX_FPU          (1 << 0)
#define CPUID_FEAT_EDX_VME          (1 << 1)
#define CPUID_FEAT_EDX_DE           (1 << 2)
#define CPUID_FEAT_EDX_PSE          (1 << 3)
#define CPUID_FEAT_EDX_TSC          (1 << 4)
#define CPUID_FEAT_EDX_MSR          (1 << 5)
#define CPUID_FEAT_EDX_PAE          (1 << 6)
#define CPUID_FEAT_EDX_MCE          (1 << 7)
#define CPUID_FEAT_EDX_CX8          (1 << 8)
#define CPUID_FEAT_EDX_APIC         (1 << 9)
#define CPUID_FEAT_EDX_SEP          (1 << 11)
#define CPUID_FEAT_EDX_MTRR         (1 << 12)
#define CPUID_FEAT_EDX_PGE          (1 << 13)
#define CPUID_FEAT_EDX_MCA          (1 << 14)
#define CPUID_FEAT_EDX_CMOV         (1 << 15)
#define CPUID_FEAT_EDX_PAT          (1 << 16)
#define CPUID_FEAT_EDX_PSE36        (1 << 17)
#define CPUID_FEAT_EDX_PSN          (1 << 18)
#define CPUID_FEAT_EDX_CLF          (1 << 19)
#define CPUID_FEAT_EDX_DTES         (1 << 21)
#define CPUID_FEAT_EDX_ACPI         (1 << 22)
#define CPUID_FEAT_EDX_MMX          (1 << 23)
#define CPUID_FEAT_EDX_FXSR         (1 << 24)
#define CPUID_FEAT_EDX_SSE          (1 << 25)
#define CPUID_FEAT_EDX_SSE2         (1 << 26)
#define CPUID_FEAT_EDX_SS           (1 << 27)
#define CPUID_FEAT_EDX_HTT          (1 << 28)
#define CPUID_FEAT_EDX_TM1          (1 << 29)
#define CPUID_FEAT_EDX_IA64         (1 << 30)
#define CPUID_FEAT_EDX_PBE          (1 << 31)

#if defined(C_CONTEXT)
#include <lib64/include/type.h>

#define CPUID(eax, ebx, ecx, edx) {\
    __asm__ volatile("cpuid;" \
                     :"=a"((uint32_t)(eax)), "=b"((uint32_t)(ebx)), \
                      "=c"((uint32_t)(ecx)), "=d"((uint32_t)(edx)) \
                     :"a"((uint32_t)(eax)) \
                     :); \
}

void
check_basic_cpu_features(void);

#endif
#endif
