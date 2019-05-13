/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_VMCS_H
#define _VMX_VMCS_H
#include <lib64/include/type.h>

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
load_vmcs(struct vmcs_blob * vm);

#endif
