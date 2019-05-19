/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_misc.h>
#include <memory/include/paging.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>
#include <x86_64/include/cpu_state.h>
#include <x86_64/include/gdt.h>

int
pre_initialize_vmcs(struct vmcs_blob * vm)
{
    memset(vm, 0x0, sizeof(struct vmcs_blob));
    vm->regions.guest_region = get_physical_page();
    ASSERT(vm->regions.guest_region);
    ASSERT(vm->regions.guest_region == pa(vm->regions.guest_region));
    return ERROR_OK;
}


static int
vmx_write(uint64_t encoding, uint64_t value)
{
    uint64_t rflag = 0;
    __asm__ volatile("vmwrite %%rdx, %%rax;"
                     "pushfq;"
                     "popq %[RFLAG];"
                     :[RFLAG]"=m"(rflag)
                     :"d"(value), "a"(encoding)
                     :"memory", "cc");
    return (rflag & RFLAG_FLAG_CARRY || rflag & RFLAG_FLAG_ZERO) ?
               -ERROR_INVALID : ERROR_OK;
}

__attribute__((unused))
static uint64_t
vmx_read(uint64_t encoding)
{
    uint64_t value = 0;
    __asm__ volatile("vmread %%rax, %[VALUE];"
                     :[VALUE]"=m"(value)
                     :"a"(encoding)
                     :"memory");
    return value;
}

static int
vmx_load(uint64_t vmcs_region)
{
    uint64_t rflag = 0;
    __asm__ volatile("vmptrld %[VMCS];"
                     "pushfq;"
                     "popq %[RFLAG];"
                     :[RFLAG]"=m"(rflag)
                     :[VMCS]"m"(vmcs_region)
                     :"memory", "cc");
    return (rflag & RFLAG_FLAG_CARRY || rflag & RFLAG_FLAG_ZERO) ?
               -ERROR_INVALID : ERROR_OK;
}

void
initialize_vmcs_host_state(struct vmcs_blob *vm)
{
#define _(encoding, value) {\
    vmx_write((encoding), (value)); \
    ASSERT(vmx_read((encoding)) == (value)); \
}
    _(HOST_CR0, get_cr0());
    _(HOST_CR3, get_cr3());
    _(HOST_CR4, get_cr4());
    _(HOST_ES_SELECTOR, KERNEL_DATA_SELECTOR);
    _(HOST_CS_SELECTOR, KERNEL_CODE_SELECTOR);
    _(HOST_SS_SELECTOR, KERNEL_DATA_SELECTOR);
    _(HOST_DS_SELECTOR, KERNEL_DATA_SELECTOR);
    _(HOST_FS_SELECTOR, KERNEL_DATA_SELECTOR);
    _(HOST_GS_SELECTOR, KERNEL_DATA_SELECTOR);
    _(HOST_TR_SELECTOR, TSS0_SELECTOR);
#undef _
}
int 
initialize_vmcs(struct vmcs_blob * vm)
{
    // load the vmcs, make the vmcs active and current, but not launched.
    uint32_t * dword_ptr = (uint32_t *)vm->regions.guest_region;
    dword_ptr[0] = get_vmx_revision_id();
    int rc = vmx_load(vm->regions.guest_region);
    LOG_INFO("vmx load vmcx:0x%x %s\n",
             vm->regions.guest_region,
             rc == ERROR_OK ? "successful" : "unsuccessful");
    if (rc != ERROR_OK) {
        return -ERROR_INVALID;
    }

    // Initialize host-state fields
    initialize_vmcs_host_state(vm);
    return ERROR_OK;
}

