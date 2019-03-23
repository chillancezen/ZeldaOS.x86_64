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
#endif
