/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_VMCS_H
#define _VMX_VMCS_H
#include <lib64/include/type.h>

// VMCS HOST STATE FIELDS: see Intel SDM Volume 3, Appendix B.1.3,
// Appendix B.2.4, Appendix B.3.4 and Appendix 4.4.4

#define HOST_ES_SELECTOR 0x00000C00
#define HOST_CS_SELECTOR 0x00000C02
#define HOST_SS_SELECTOR 0x00000C04
#define HOST_DS_SELECTOR 0x00000C06
#define HOST_FS_SELECTOR 0x00000C08
#define HOST_GS_SELECTOR 0x00000C0A
#define HOST_TR_SELECTOR 0x00000C0C
#define HOST_IA32_PAT_FULL 0x00002C00
#define HOST_IA32_PAT_HIGH 0x00002C00
#define HOST_IA32_EFER_FULL 0x00002C02
#define HOST_IA32_EFER_HIGH 0x00002C03
#define HOST_IA32_PERF_GLOBAL_CTRL_FULL 0x00002C04
#define HOST_IA32_PERF_GLOBAL_CTRL_HIGH 0x00002C05
#define HOST_IA32_SYSENTER_CS 0x00004C00
#define HOST_CR0 0x00006C00
#define HOST_CR3 0x00006C02
#define HOST_CR4 0x00006C04
#define HOST_FS_BASE 0x00006C06
#define HOST_GS_BASE 0x00006C08
#define HOST_TR_BASE 0x00006C0A
#define HOST_GDTR_BASE 0x00006C08
#define HOST_IDTR_BASE 0x00006C0E
#define HOST_IA32_SYSENTER_ESP 0x00006C10
#define HOST_IA32_SYSENTER_EIP 0x00006C12
#define HOST_RSP 0x00006C14
#define HOST_RIP 0x00006C16


struct vmcs_region {
    uint64_t guest_region;
    uint64_t io_bitmap_region0;
    uint64_t io_bitmap_region1;
    uint64_t msr_bitmap_region;
    uint64_t virtual_apic_region;
};

struct vmcs_blob {
    struct vmcs_region regions;
};

int
pre_initialize_vmcs(struct vmcs_blob * vm);


int
initialize_vmcs(struct vmcs_blob * vm);

#endif
