/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_EPT_H
#define _VMX_EPT_H
#include <vm_monitor/include/vmx_vmcs.h>



struct ept_pml4e{
    uint64_t read_access:1;
    uint64_t write_access:1;
    uint64_t instruction_fetchable:1;
    uint64_t reserved0:5;
    uint64_t accessed:1;
    uint64_t reserved1:1;
    uint64_t usermode_instruction_fetchable:1;
    uint64_t reserved2:1;
    uint64_t ept_pdpt:40;
    uint64_t reserved3:12;
}__attribute__((packed));

struct ept_pdpe {
    uint64_t read_access:1;
    uint64_t write_access:1;
    uint64_t instruction_fetchable:1;
    uint64_t reserved0:5;
    uint64_t accessed:1;
    uint64_t reserved1:1;
    uint64_t usermode_instruction_fetchable:1;
    uint64_t reserved2:1;
    uint64_t ept_pdt:40;
    uint64_t reserved3:12;
}__attribute__((packed));


struct ept_pde {
    uint64_t read_access:1;
    uint64_t write_access:1;
    uint64_t instruction_fetchable:1;
    uint64_t reserved0:5;
    uint64_t accessed:1;
    uint64_t reserved1:1;
    uint64_t usermode_instruction_fetchable:1;
    uint64_t reserved2:1;
    uint64_t ept_pt:40;
    uint64_t reserved3:12;
}__attribute__((packed));

struct ept_pte {
    uint64_t read_access:1;
    uint64_t write_access:1;
    uint64_t instruction_fetchable:1;
    uint64_t ept_memory_type:3;
    uint64_t ignore_pat_memory_type:1;
    uint64_t reserved0:1;
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t usermode_instruction_fetchable:1;
    uint64_t reserved1:1;
    uint64_t ept_4k_page:40;
    uint64_t reserved2:12;
}__attribute__((packed));


uint64_t
setup_basic_physical_memory(uint64_t addr_low, uint64_t addr_high);

void
setup_io_memory(uint64_t ept_pml4_base, uint64_t guest_physical_page);

uint64_t
guestpa_to_hostpa(uint64_t ept_base, uint64_t guestpa);


#define PAGE_SYSTEM_MEMORY 0x0
#define PAGE_SYSTEM_IOMEMORY 0x1

#endif
