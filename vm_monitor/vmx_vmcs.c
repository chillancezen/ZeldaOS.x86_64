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
#include <x86_64/include/lapic.h>
#include <x86_64/include/tss.h>
#include <x86_64/include/msr.h>

#define VMXWRITE(encoding, value) {                                            \
    uint64_t __value = (value);                                                \
    ASSERT(vmx_write((encoding), __value) == ERROR_OK);                        \
    ASSERT(vmx_read((encoding)) == __value);                                   \
}


extern void * vm_exit_handler;

int
pre_initialize_vmcs(struct vmcs_blob * vm)
{
#define _(_va) {                                                               \
    ASSERT(_va);                                                               \
    ASSERT(_va == pa(_va));                                                    \
}

    memset(vm, 0x0, sizeof(struct vmcs_blob));
    vm->regions.guest_region = get_physical_page();
    vm->regions.io_bitmap_region0 = get_physical_page();
    vm->regions.io_bitmap_region1 = get_physical_page();
    vm->host_stack = get_physical_pages(HOST_STACK_NR_PAGES);
    _(vm->regions.guest_region);
    _(vm->regions.io_bitmap_region0);
    _(vm->regions.io_bitmap_region1);
    _(vm->host_stack);
    return ERROR_OK;
#undef _
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

static void
initialize_vmcs_host_state(struct vmcs_blob *vm)
{
    VMXWRITE(HOST_CR0, get_cr0());
    VMXWRITE(HOST_CR3, get_cr3());
    VMXWRITE(HOST_CR4, get_cr4());
    VMXWRITE(HOST_ES_SELECTOR, KERNEL_DATA_SELECTOR);
    VMXWRITE(HOST_CS_SELECTOR, KERNEL_CODE_SELECTOR);
    VMXWRITE(HOST_SS_SELECTOR, KERNEL_DATA_SELECTOR);
    VMXWRITE(HOST_DS_SELECTOR, KERNEL_DATA_SELECTOR);
    VMXWRITE(HOST_FS_SELECTOR, KERNEL_DATA_SELECTOR);
    // here Only BSP enters vmx operation, orther cpu MUST NOT enter vmx mode
    // using this helper function.
    VMXWRITE(HOST_GS_SELECTOR, KERNEL_DATA_SELECTOR);
    VMXWRITE(HOST_TR_SELECTOR, TSS0_SELECTOR);
    // We do not enable systemcall in cr4.
    VMXWRITE(HOST_IA32_SYSENTER_CS, 0x0);
    VMXWRITE(HOST_IA32_SYSENTER_ESP, 0x0);
    VMXWRITE(HOST_IA32_SYSENTER_EIP, 0x0);

    VMXWRITE(HOST_IDTR_BASE, get_idtr_base());
    VMXWRITE(HOST_GDTR_BASE, get_gdtr_base());
    VMXWRITE(HOST_FS_BASE, get_fs_base());
    VMXWRITE(HOST_GS_BASE, get_gs_base());
    VMXWRITE(HOST_TR_BASE, (uint64_t)get_tss_base());
    VMXWRITE(HOST_RSP, vm->host_stack);
    VMXWRITE(HOST_RSP, (uint64_t)&vm_exit_handler);
}

static void
initialize_vmcs_guest_state(struct vmcs_blob *vm)
{
    VMXWRITE(GUEST_ES_SELECTOR, 0x0);
    VMXWRITE(GUEST_CS_SELECTOR, 0x0);
    VMXWRITE(GUEST_DS_SELECTOR, 0x0);
    VMXWRITE(GUEST_FS_SELECTOR, 0x0);
    VMXWRITE(GUEST_GS_SELECTOR, 0x0);
    VMXWRITE(GUEST_SS_SELECTOR, 0x0);
    VMXWRITE(GUEST_TR_SELECTOR, 0x0);
    VMXWRITE(GUEST_LDTR_SELECTOR, 0x0);
    VMXWRITE(GUEST_CS_BASE, 0x0);
    VMXWRITE(GUEST_DS_BASE, 0x0);
    VMXWRITE(GUEST_ES_BASE, 0x0);
    VMXWRITE(GUEST_FS_BASE, 0x0);
    VMXWRITE(GUEST_GS_BASE, 0x0);
    VMXWRITE(GUEST_SS_BASE, 0x0);
    VMXWRITE(GUEST_LDTR_BASE, 0x0);
    VMXWRITE(GUEST_IDTR_BASE, 0x0);
    VMXWRITE(GUEST_GDTR_BASE, 0x0);
    VMXWRITE(GUEST_TR_BASE, 0x0);
    VMXWRITE(GUEST_CS_LIMIT, 0xffff);
    VMXWRITE(GUEST_DS_LIMIT, 0xffff);
    VMXWRITE(GUEST_ES_LIMIT, 0xffff);
    VMXWRITE(GUEST_FS_LIMIT, 0xffff);
    VMXWRITE(GUEST_GS_LIMIT, 0xffff);
    VMXWRITE(GUEST_SS_LIMIT, 0xffff);
    VMXWRITE(GUEST_LDTR_LIMIT, 0xffff);
    VMXWRITE(GUEST_TR_LIMIT, 0xffff);
    VMXWRITE(GUEST_GDTR_LIMIT, 0xffff);
    VMXWRITE(GUEST_IDTR_LIMIT, 0xffff);
    // see Figure 3-8. Segment Descriptor for the access right
    // and see table 24-2 for the format of the access right.
    #define DATA_ACCESS_RIGHT (0x3 | 1 << 4 | 1 << 7)
    #define CODE_ACCESS_RIGHT (0xa | 1 << 4 | 1 << 7 | 1 << 13)
    #define LDTR_ACCESS_RIGHT (0x2 | 1 << 7)
    #define TR_ACCESS_RIGHT (0xb | 1 << 7)
    VMXWRITE(GUEST_CS_ACCESS_RIGHT, CODE_ACCESS_RIGHT);
    VMXWRITE(GUEST_DS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    VMXWRITE(GUEST_ES_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    VMXWRITE(GUEST_FS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    VMXWRITE(GUEST_GS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    VMXWRITE(GUEST_SS_ACCESS_RIGHT, DATA_ACCESS_RIGHT);
    VMXWRITE(GUEST_LDTR_ACCESS_RIGHT, LDTR_ACCESS_RIGHT);
    VMXWRITE(GUEST_TR_ACCESS_RIGHT, TR_ACCESS_RIGHT);
    VMXWRITE(GUEST_INTERRUPTIBILITY_STATE, 0x0);
    VMXWRITE(GUEST_ACTIVITY_STATE, 0x0);
    VMXWRITE(GUEST_CR0, CR0_NE);
    VMXWRITE(GUEST_CR3, 0x0);
    VMXWRITE(GUEST_CR4, 0x0);
    VMXWRITE(GUEST_DR7, 0x0);
    VMXWRITE(GUEST_RSP, 0x0);
    VMXWRITE(GUEST_RIP, 0x0);
    #define RFLAG_RESERVED (1 << 1)
    VMXWRITE(GUEST_RFLAG, RFLAG_RESERVED);
    VMXWRITE(GUEST_VMCS_LINK_POINTER_LOW, 0xffffffff);
    VMXWRITE(GUEST_VMCS_LINK_POINTER_HIGH, 0xffffffff);
}

static int
validate_capability_dword(uint64_t target, uint32_t allowed0, uint32_t allowed1)
{
    int idx = 0;
    for (idx = 0; idx < 32; idx++) {
        uint32_t mask = 1 << idx;
        int target_is_set = !!(target & mask);
        int allowed0_is_set = !!(allowed0 & mask);
        int allowed1_is_set = !!(allowed1 & mask);
        if ((allowed0_is_set && !target_is_set) ||
            (!allowed1_is_set && target_is_set)) {
            return -ERROR_INVALID;
        }
    }
    return ERROR_OK;
}

static uint32_t
fix_reserved_1_bits(uint32_t target, uint32_t allowed0)
{
    int idx = 0;
    for (idx = 0; idx < 32; idx++) {
        uint32_t mask = 1 << idx;
        int target_is_set = !!(target & mask);
        int allowed0_is_set = !!(allowed0 & mask);
        if (allowed0_is_set && !target_is_set) {
            target |= mask;
        }
    }
    return target;
}

static uint32_t
fix_reserved_0_bits(uint32_t target, uint32_t allowed1)
{
    int idx = 0;
    for (idx = 0; idx < 32; idx++) {
        uint32_t mask = 1 << idx;
        int target_is_set = !!(target & mask);
        int allowed1_is_set = !!(allowed1 & mask);
        if (!allowed1_is_set && target_is_set) {
            target &= ~mask;
        }
    }
    return target;
}

static void
initialize_vmcs_pinbased_control(struct vmcs_blob *vm)
{
    uint32_t pinbased_msr_eax, pinbased_msr_edx;
    RDMSR(IA32_VMX_PINBASED_CTLS_MSR, &pinbased_msr_eax, &pinbased_msr_edx);
    // See Table 24-5 and Appendix A.3.1
    LOG_DEBUG("vmx pinbased.msr.eax:0x%x\n", pinbased_msr_eax);
    LOG_DEBUG("vmx pinbased.msr.edx:0x%x\n", pinbased_msr_edx);
    uint32_t pinbased_vm_execution_ctrl = 0;
    // External Interrupt causes a VM EXIT
    pinbased_vm_execution_ctrl |= 1;
    pinbased_vm_execution_ctrl = fix_reserved_1_bits(pinbased_vm_execution_ctrl,
                                                     pinbased_msr_eax);
    pinbased_vm_execution_ctrl = fix_reserved_0_bits(pinbased_vm_execution_ctrl,
                                                     pinbased_msr_edx);
    ASSERT(validate_capability_dword(pinbased_vm_execution_ctrl,
                                     pinbased_msr_eax, pinbased_msr_edx) ==
           ERROR_OK);
    LOG_DEBUG("vmx pinbased.msr.dword:0x%x\n", pinbased_vm_execution_ctrl);
    VMXWRITE(CTLS_PIN_BASED_VM_EXECUTION, pinbased_vm_execution_ctrl);
}

static void
initialize_vmcs_procbased_control(struct vmcs_blob *vm)
{
    
}


static void
initialize_vmcs_io_bitmap(struct vmcs_blob * vm)
{
    // there are two 4k aligned pages to define the ioport behavior
    // See 24.6.4 and 25.1.3 for more.
    #define IOPORT_TO_REGION(vm, _port)                                        \
        (((_port) >= 0 && (_port) <= 0x7FFF) ? (vm)->regions.io_bitmap_region0:\
         (vm)->regions.io_bitmap_region1)

    #define IOPORT_IS_SET(vm, _port) ({                                        \
        uint8_t * _ptr = (uint8_t *)(uint64_t)IOPORT_TO_REGION((vm), (_port)); \
        uint16_t __port = ((_port) >= 0x8000) ? _port - 0x8000 : _port;        \
        uint16_t __byte_index = __port >> 3;                                   \
        uint8_t __bit_index = __port & 0x7;                                    \
        !!(_ptr[__byte_index] & (1 << __bit_index));                           \
    })

    #define SET_IOPORT(vm, _port) {                                            \
        uint8_t * _ptr = (uint8_t *)(uint64_t)IOPORT_TO_REGION((vm), (_port)); \
        uint16_t __port = ((_port) >= 0x8000) ? _port - 0x8000 : _port;        \
        uint16_t __byte_index = __port >> 3;                                   \
        uint8_t __bit_index = __port & 0x7;                                    \
        _ptr[__byte_index] |= 1 << __bit_index;                                \
    }
    
#define _(_port) {                                                             \
    SET_IOPORT(vm, _port);                                                     \
    ASSERT(IOPORT_IS_SET(vm, _port));                                          \
}
    // Enable serial ports
    _(0x3f8);
    _(0x3f9);
    _(0x3fa);
    _(0x3fb);
    _(0x3fc);
    _(0x3fd);
    // Enable keyboard ports
    _(0x60);
    _(0x64);
#undef _
}


static void
initialize_vmcs_execution_control(struct vmcs_blob *vm)
{
    initialize_vmcs_pinbased_control(vm);
    initialize_vmcs_io_bitmap(vm);
    initialize_vmcs_procbased_control(vm);
}

int 
initialize_vmcs(struct vmcs_blob * vm)
{
    // load the vmcs, make the vmcs active and current, but not launched.
    uint32_t * dword_ptr = (uint32_t *)vm->regions.guest_region;
    dword_ptr[0] = get_vmx_revision_id();
    int rc = vmx_load(vm->regions.guest_region);
    LOG_INFO("vmx load vmcs:0x%x %s\n",
             vm->regions.guest_region,
             rc == ERROR_OK ? "successful" : "unsuccessful");
    if (rc != ERROR_OK) {
        return -ERROR_INVALID;
    }

    // Initialize host-state fields
    initialize_vmcs_host_state(vm);
    initialize_vmcs_guest_state(vm);
    initialize_vmcs_execution_control(vm);
    return ERROR_OK;
}

