/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_misc.h>
#include <memory/include/paging.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>

int
pre_initialize_vmcs(struct vmcs_blob * vm)
{
    memset(vm, 0x0, sizeof(struct vmcs_blob));
    vm->regions.guest_region = get_physical_page();
    ASSERT(vm->regions.guest_region);

    return ERROR_OK;
}



int 
load_vmcs(struct vmcs_blob * vm)
{
    // load the vmcs, make the vmcs active and current, but not launched.
    uint32_t * dword_ptr = (uint32_t *)vm->regions.guest_region;
    dword_ptr[0] = get_vmx_revision_id();
    __asm__ volatile("cli;"
                     "hlt;"
                     "vmptrld %[VMCS];"
                     :
                     :[VMCS]"m"(vm->regions.guest_region)
                     :"memory", "cc");
    return ERROR_OK;
}

