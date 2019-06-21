/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_misc.h>
#include <vm_monitor/include/vmx_ept.h>
#include <memory/include/paging.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>
#include <x86_64/include/cpu_state.h>
#include <x86_64/include/gdt.h>
#include <x86_64/include/lapic.h>
#include <x86_64/include/tss.h>
#include <x86_64/include/msr.h>
#include <x86_64/include/per_cpu.h>

#define VMXWRITE(encoding, value) {                                            \
    uint64_t __value = (value);                                                \
    ASSERT(vmx_write((encoding), __value) == ERROR_OK);                        \
    ASSERT(vmx_read((encoding)) == __value);                                   \
    LOG_TRIVIA("VMX Write: %s:0x%x value:0x%x\n",                              \
               #encoding, encoding, __value);                                  \
}


extern void * vm_exit_handler;
extern void * _guest64_start;
extern void * _guest64_end;
static DECLARE_PER_CPU_VARIABLE(struct vmcs_blob *, current_vm);

int
pre_initialize_vmcs(struct vmcs_blob * vm)
{
#define _(_va) {                                                               \
    ASSERT(_va);                                                               \
    memset((void *)_va, 0x0, PAGE_SIZE_4K);                                    \
    ASSERT(_va == pa(_va));                                                    \
}

    memset(vm, 0x0, sizeof(struct vmcs_blob));
    vm->regions.guest_region = get_physical_page();
    vm->regions.io_bitmap_region0 = get_physical_page();
    vm->regions.io_bitmap_region1 = get_physical_page();
    vm->regions.vm_exit_store_msr_region = get_physical_page();
    vm->regions.vm_exit_load_msr_region = get_physical_page();
    vm->host_stack = get_physical_pages(HOST_STACK_NR_PAGES);
    vm->serial_line_buffer = (uint8_t *)get_physical_page();
    _(vm->regions.guest_region);
    _(vm->regions.io_bitmap_region0);
    _(vm->regions.io_bitmap_region1);
    _(vm->regions.vm_exit_store_msr_region);
    _(vm->regions.vm_exit_load_msr_region);
    _(vm->host_stack);
    _((uint64_t)vm->serial_line_buffer);
    vm->serial_line_iptr = 0;
    return ERROR_OK;
#undef _
}


int
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

uint64_t
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
vmx_launch(void)
{
    uint64_t rflag;
    __asm__ volatile("vmlaunch;"
                     "pushfq;"
                     "popq %[RFLAG];"
                     :[RFLAG]"=m"(rflag)
                     :
                     :"cc");
    int cf_is_set = !!(rflag & RFLAG_FLAG_CARRY);
    int zf_is_set = !!(rflag & RFLAG_FLAG_ZERO);
    LOG_TRIVIA("vmlaunch cf is set:%d\n", cf_is_set);
    LOG_TRIVIA("vmlaunch zf is set:%d\n", zf_is_set);
    // According to 30.2, if ZF of RFLAGS is set, it fails with an error number
    // which is stored in the `VM-Instruction error` field.
    uint64_t vmx_error = vmx_read(RDONLY_VM_INSTRUCTION_ERROR);
    LOG_DEBUG("vmx instruction error number:0x%x\n", vmx_error);
    __not_reach();
}

__attribute__((unused)) static int
vmx_clear(uint64_t vmcs_region)
{
    uint64_t rflag;
    __asm__ volatile("vmclear %[REGION];"
                     :[RFLAG]"=m"(rflag)
                     :[REGION]"m"(vmcs_region)
                     :"cc", "memory");
    return (rflag & RFLAG_FLAG_CARRY || rflag & RFLAG_FLAG_ZERO) ?
               -ERROR_INVALID : ERROR_OK;
}

static void
initialize_vmcs_host_state(struct vmcs_blob *vm)
{
    uint32_t eax, edx;
    {
        RDMSR(IA32_VMX_CR0_FIXED0_MSR, &eax, &edx);
        VMXWRITE(HOST_CR0, eax | get_cr0());
    }
    VMXWRITE(HOST_CR3, get_cr3());
    {
        RDMSR(IA32_VMX_CR4_FIXED0_MSR, &eax, &edx);
        VMXWRITE(HOST_CR4, eax | get_cr4());
    }
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
    VMXWRITE(HOST_RIP, (uint64_t)&vm_exit_handler);
    VMXWRITE(HOST_IA32_EFER, get_efer());
}

static void
initialize_vmcs_guest_state(struct vmcs_blob *vm)
{
    uint64_t eax, edx;
    VMXWRITE(GUEST_ES_SELECTOR, 0x0);
    VMXWRITE(GUEST_CS_SELECTOR, 0x100);
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
    // FIXED: the access right do impact my vm entry.
    #define DATA_ACCESS_RIGHT (0x3 | 1 << 4 | 1 << 7)
    #define CODE_ACCESS_RIGHT (0x3 | 1 << 4 | 1 << 7 | 1 << 13)
    #define LDTR_ACCESS_RIGHT (0x2 | 1 << 7)
    #define TR_ACCESS_RIGHT (0x3 | 1 << 7)
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
    {
        RDMSR(IA32_VMX_CR0_FIXED0_MSR, &eax, &edx);
        eax &= ~(1 << 0); // disable PE
        eax &= ~(1 << 31); // disable PG
        VMXWRITE(GUEST_CR0, eax | ((uint64_t)edx) << 32);
    }
    VMXWRITE(GUEST_CR3, 0x0);
    {
        RDMSR(IA32_VMX_CR4_FIXED0_MSR, &eax, &edx);
        VMXWRITE(GUEST_CR4, eax | ((uint64_t)edx) << 32);
    }
    VMXWRITE(GUEST_DR7, 0x0);
    VMXWRITE(GUEST_RSP, 0x0);
    VMXWRITE(GUEST_RIP, 0x0); // guest cs:ip -> 0x100:0
    #define RFLAG_RESERVED (1 << 1)
    VMXWRITE(GUEST_RFLAG, RFLAG_RESERVED);
    VMXWRITE(GUEST_VMCS_LINK_POINTER_LOW, 0xffffffff);
    VMXWRITE(GUEST_VMCS_LINK_POINTER_HIGH, 0xffffffff);

    VMXWRITE(GUEST_IA32_EFER, 0x0);
}

static int
validate_vmx_capability(uint64_t target, uint32_t allowed0, uint32_t allowed1)
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
    //pinbased_vm_execution_ctrl |= 1;
    pinbased_vm_execution_ctrl = fix_reserved_1_bits(pinbased_vm_execution_ctrl,
                                                     pinbased_msr_eax);
    pinbased_vm_execution_ctrl = fix_reserved_0_bits(pinbased_vm_execution_ctrl,
                                                     pinbased_msr_edx);
    ASSERT(validate_vmx_capability(pinbased_vm_execution_ctrl,
                                   pinbased_msr_eax, pinbased_msr_edx) ==
           ERROR_OK);
    VMXWRITE(CTLS_PIN_BASED_VM_EXECUTION, pinbased_vm_execution_ctrl);
}

static void
initialize_vmcs_procbased_control(struct vmcs_blob *vm)
{
    uint32_t vpid_and_ept_msr_eax, vpid_and_ept_msr_edx;
    uint32_t pri_procbased_msr_eax, pri_procbased_msr_edx;
    uint32_t sec_procbased_msr_eax, sec_procbased_msr_edx;
    // See Appendix A.3.2, Appendix A.10 and Appendix A.3.3
    RDMSR(IA32_VMX_EPT_VPID_CAP_MSR, &vpid_and_ept_msr_eax,
          &vpid_and_ept_msr_edx);
    RDMSR(IA32_VMX_PRI_PROCBASED_CTLS_MSR, &pri_procbased_msr_eax,
          &pri_procbased_msr_edx);
    RDMSR(IA32_VMX_SEC_PROCBASED_CTLS_MSR, &sec_procbased_msr_eax,
          &sec_procbased_msr_edx);
    LOG_DEBUG("vmx ept and vpid cap msr.eax:0x%x\n", vpid_and_ept_msr_eax);
    LOG_DEBUG("vmx ept and vpid cap msr.edx:0x%x\n", vpid_and_ept_msr_edx);
    LOG_DEBUG("vmx primary procbased.msr.eax:0x%x\n", pri_procbased_msr_eax);
    LOG_DEBUG("vmx primary procbased.msr.edx:0x%x\n", pri_procbased_msr_edx);
    LOG_DEBUG("vmx second procbased.msr.eax:0x%x\n", sec_procbased_msr_eax);
    LOG_DEBUG("vmx second procbased.msr.edx:0x%x\n", sec_procbased_msr_edx);
    // Examine basic ept and vpid capability.
    ASSERT(vpid_and_ept_msr_eax & 0x1);
    //ASSERT(vpid_and_ept_msr_eax & (1 << 8));
    ASSERT(vpid_and_ept_msr_eax & (1 << 14));
    ASSERT(vpid_and_ept_msr_eax & (1 << 21));
    ASSERT(vpid_and_ept_msr_edx & 0x1);
    {
        // primary process based execution control, See Table 24-6
        uint32_t pri_procbase_ctls = 0;
        pri_procbase_ctls |= 1 << 7; // Hlt causes vm exit
        pri_procbase_ctls |= 1 << 9; // INVLPG causes vm exit
        pri_procbase_ctls |= 1 << 15; // CR3-load causes vm exit
        pri_procbase_ctls |= 1 << 16; // CR3-store causes vm exit
        pri_procbase_ctls |= 1 << 24; // Unconditional IO exiting
        //pri_procbase_ctls |= 1 << 25; // Use IO bitmap
        pri_procbase_ctls |= 1 << 30; // PAUSE causes vm exit
        pri_procbase_ctls |= 1 << 31; // activate secondary controls
        pri_procbase_ctls = fix_reserved_1_bits(pri_procbase_ctls,
                                                pri_procbased_msr_eax);
        pri_procbase_ctls = fix_reserved_0_bits(pri_procbase_ctls,
                                                pri_procbased_msr_edx);
        ASSERT(validate_vmx_capability(pri_procbase_ctls, pri_procbased_msr_eax,
                                       pri_procbased_msr_edx) == ERROR_OK);
        VMXWRITE(CTLS_PRI_PROC_BASED_VM_EXECUTION, pri_procbase_ctls);
    }
    {
        // second process based execution control, see Table 24-7
        uint32_t sec_procbase_ctls = 0;
        sec_procbase_ctls |= 1 << 1; // use EPT
        //sec_procbase_ctls |= 1 << 2; // descriptor-table exiting:GDT/LDT/IDT/TR
        sec_procbase_ctls |= 1 << 5; // enable VPID
        sec_procbase_ctls |= 1 << 7; // unrestricted guest
        sec_procbase_ctls = fix_reserved_1_bits(sec_procbase_ctls,
                                                sec_procbased_msr_eax);
        sec_procbase_ctls = fix_reserved_0_bits(sec_procbase_ctls,
                                                sec_procbased_msr_edx);
        ASSERT(validate_vmx_capability(sec_procbase_ctls, sec_procbased_msr_eax,
                                       sec_procbased_msr_edx) == ERROR_OK);
        VMXWRITE(CTLS_SEC_PROC_BASED_VM_EXECUTION, sec_procbase_ctls);
    }
    {
        // Exception bitmap, see 6.15 for all the exception reference.
        // we want no exception cause vm exits here
        uint32_t exception_bitmap = 0;
        VMXWRITE(CTLS_EXCEPTION_BITMAP, exception_bitmap);
        // Set the IO bitmap
        VMXWRITE(CTLS_IO_BITMAP_A, vm->regions.io_bitmap_region0);
        VMXWRITE(CTLS_IO_BITMAP_B, vm->regions.io_bitmap_region1);
        //Set the CR3 target count, no CR3 target causes vm exit.
        VMXWRITE(CTLS_CR3_TARGET_COUNT, 0x0);
    }
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


static void
initialize_vmcs_vm_exit_msr(struct vmcs_blob * vm)
{
    memset((void *)vm->regions.vm_exit_store_msr_region, 0x0, PAGE_SIZE_4K);
    memset((void *)vm->regions.vm_exit_load_msr_region, 0x0, PAGE_SIZE_4K);
    uint32_t predefined_msrs[0];// = {IA32_EFER_MSR};
    int nr_msrs = sizeof(predefined_msrs) / sizeof(uint32_t);
    vm->regions.vm_exit_load_msr_count = nr_msrs;
    vm->regions.vm_exit_store_msr_count = nr_msrs;
    int index = 0;
    for (index = 0; index < nr_msrs; index++) {
        struct vmcs_msr_blob * vmcs_msr =
            ((struct vmcs_msr_blob *)vm->regions.vm_exit_store_msr_region) +
            index;
        vmcs_msr->index = predefined_msrs[index];
        vmcs_msr->reserved = 0;
        RDMSR(vmcs_msr->index, &vmcs_msr->msr_eax, &vmcs_msr->msr_edx);

        vmcs_msr =
            ((struct vmcs_msr_blob *)vm->regions.vm_exit_load_msr_region) +
            index;
        vmcs_msr->index = predefined_msrs[index];
        vmcs_msr->reserved = 0;
        RDMSR(vmcs_msr->index, &vmcs_msr->msr_eax, &vmcs_msr->msr_edx);
    }
    VMXWRITE(CTLS_VM_EXIT_MSR_STORE_COUNT, vm->regions.vm_exit_store_msr_count);
    VMXWRITE(CTLS_VM_EXIT_MSR_LOAD_COUNT, vm->regions.vm_exit_load_msr_count);
    VMXWRITE(CTLS_VM_EXIT_MSR_LOAD, vm->regions.vm_exit_load_msr_region);
    VMXWRITE(CTLS_VM_EXIT_MSR_STORE, vm->regions.vm_exit_store_msr_region);
}

static void
initialize_vmcs_vm_exit_control(struct vmcs_blob * vm)
{
    uint32_t vm_exit_msr_eax, vm_exit_msr_edx;
    RDMSR(IA32_VMX_VM_EXIT_CTLS_MSR, &vm_exit_msr_eax, &vm_exit_msr_edx);
    LOG_DEBUG("vmx vm.exit.ctls.eax:0x%x\n", vm_exit_msr_eax);
    LOG_DEBUG("vmx vm.exit.ctls.edx:0x%x\n", vm_exit_msr_edx);
    uint32_t vm_exit_ctls = 0;
    vm_exit_ctls |= 1 << 9; // VM exit to 64-bit long mode.
    vm_exit_ctls |= 1 << 15; // ACK external interrupts.
    vm_exit_ctls |= 1 << 21; // Load IA32_EFER on vm-exit
    vm_exit_ctls = fix_reserved_1_bits(vm_exit_ctls, vm_exit_msr_eax);
    vm_exit_ctls = fix_reserved_0_bits(vm_exit_ctls, vm_exit_msr_edx);
    ASSERT(validate_vmx_capability(vm_exit_ctls, vm_exit_msr_eax,
                                   vm_exit_msr_edx) == ERROR_OK);
    VMXWRITE(CTLS_VM_EXIT, vm_exit_ctls);
    initialize_vmcs_vm_exit_msr(vm);
}


static void
initialize_vmcs_vm_entry_msr(struct vmcs_blob * vm)
{
    // The MSRs are loaded on vm entry
    // they are initialized in initialize_vmcs_vm_exit_msr()
    VMXWRITE(CTLS_VM_ENTRY_MSR_LOAD_COUNT, vm->regions.vm_exit_store_msr_count);
    VMXWRITE(CTLS_VM_ENTRY_MSR_LOAD, vm->regions.vm_exit_store_msr_region);
}

static void
initialize_vmcs_vm_entry_control(struct vmcs_blob * vm)
{
    uint32_t vm_entry_msr_eax, vm_entry_msr_edx;
    RDMSR(IA32_VMX_VM_ENTRY_CTLS_MSR, &vm_entry_msr_eax, &vm_entry_msr_edx);
    uint32_t vm_entry_ctls = 0;
    // Do not mangle the 9th bit of the vm entry control, as the guest
    // IA32_EFER.LMA is stored here. and loaded on vmentry. 
    vm_entry_ctls |= 1 << 15; // load EFER msr on vm-entry
    vm_entry_ctls = fix_reserved_1_bits(vm_entry_ctls, vm_entry_msr_eax);
    vm_entry_ctls = fix_reserved_0_bits(vm_entry_ctls, vm_entry_msr_edx);
    ASSERT(validate_vmx_capability(vm_entry_ctls, vm_entry_msr_eax,
                                   vm_entry_msr_edx) == ERROR_OK);
    LOG_DEBUG("vmx vm.entry.ctls.eax:0x%x\n", vm_entry_msr_eax);
    LOG_DEBUG("vmx vm.entry.ctls.edx:0x%x\n", vm_entry_msr_edx);
    VMXWRITE(CTLS_VM_ENTRY, vm_entry_ctls);
    initialize_vmcs_vm_entry_msr(vm);
    // No event is injected: the 31th bit is invalid
    VMXWRITE(CTLS_VM_ENTRY_INTERRUPT_INFORMATION_FIELD, 0x0);
}
static void
initialize_vmcs_ept(struct vmcs_blob * vm)
{
    // Let's allocate the fixed 16MB to boot the guest, which is supposed to be
    // enough to accomondate the guest image
    vm->regions.ept_pml4_base = setup_basic_physical_memory(0, 16*1024*1024);

    uint64_t eptp = vm->regions.ept_pml4_base;
    eptp |= 6;// query the bit 8 of the VPID_EPT VMX CAP
    eptp |= (4 - 1) << 3; // page-walk length:4
    eptp |= 1 << 6; // enable acccessed and dirty marking
    VMXWRITE(CTLS_EPTP, eptp);
    ASSERT(vm->vpid);
    VMXWRITE(CTLS_VPID, vm->vpid);
}

static void
initialize_vmcs_ealy_state(struct vmcs_blob * vm)
{
    uint32_t eax;
    uint32_t edx;
    RDMSR(IA32_VMX_BASIC_MSR, &eax, &edx);
    // If memory type of access to vmcs is not write-back, PANIC
    ASSERT(((edx >> (50 - 32)) & 0xf) == 0x6);
}

static void
initialize_vmcs_guest_image(struct vmcs_blob * vm)
{
    uint64_t guest_image_start_addr = (uint64_t)(&_guest64_start);
    uint64_t guest_image_end_addr = (uint64_t)(&_guest64_end);
    LOG_DEBUG("vm:0x%x's image start:0x%x\n",vm, guest_image_start_addr);
    LOG_DEBUG("vm:0x%x's image end:0x%x\n",vm,  guest_image_end_addr);

    // copy the guest image into guest memory which begins at its second page
    uint64_t img_addr = guest_image_start_addr;
    uint64_t guest_addr = PAGE_SIZE_4K;
    uint64_t host_pa;
    for (; img_addr < guest_image_end_addr; img_addr += PAGE_SIZE_4K) {
        host_pa = guestpa_to_hostpa(vm->regions.ept_pml4_base, guest_addr);
        ASSERT(pa(host_pa) == host_pa);
        memcpy((void *)host_pa, (void *)img_addr, PAGE_SIZE_4K);
        // FIXED: I forgot to adjust the guest_addr, thus when the guest image
        // size exceeds 4096 bytes, my guest image in memory is ruined.
        guest_addr += PAGE_SIZE_4K;
    }
}

struct vmcs_blob *
get_current_vm(void)
{
    struct vmcs_blob ** pvm = THIS_CPU(struct vmcs_blob *, current_vm);
    return *pvm;
}

void
set_current_vm(struct vmcs_blob * vm)
{
    struct vmcs_blob ** pvm = THIS_CPU(struct vmcs_blob *, current_vm);
    *pvm = vm;
}

void
dump_vm(struct vmcs_blob * vm)
{
    // XXX: the vm must be current vm, and vmcs is marked as 'current'
    struct guest_cpu_state * vcpu = vm->vcpu;
    ASSERT(get_current_vm() == vm);
        LOG_DEBUG("dump vm:0x%x(vpid:%d)\n", vm, vm->vpid);
    LOG_DEBUG("   RAX:0x%x RBX:0x%x RCX:0x%x RDX:0x%x\n",
              vcpu->rax, vcpu->rbx, vcpu->rcx, vcpu->rdx);
    LOG_DEBUG("   R15:0x%x R14:0x%x R13:0x%x, R12:0x%x\n",
              vcpu->r15, vcpu->r14, vcpu->r13, vcpu->r12);
    LOG_DEBUG("   R11:0x%x R10:0x%x R9:0x%x R8:0x%x\n",
              vcpu->r11, vcpu->r10, vcpu->r9, vcpu->r8);
    LOG_DEBUG("   RSI:0x%x RDI:0x%x RBP:0x%x RSP:0x%x\n",
              vcpu->rsi, vcpu->rdi, vcpu->rbp, vmx_read(GUEST_RSP));
    LOG_DEBUG("   CR0:0x%x CR2:0x%x CR3:0x%x CR4:0x%x\n",
              vmx_read(GUEST_CR0), vcpu->cr2, vmx_read(GUEST_CR3),
              vmx_read(GUEST_CR4));
    LOG_DEBUG("   RIP:0x%x RFLAGS:0x%x EFER:0x%x\n",
              vmx_read(GUEST_RIP), vmx_read(GUEST_RFLAG),
              vmx_read(GUEST_IA32_EFER));
    LOG_DEBUG("   CS  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_CS_SELECTOR), vmx_read(GUEST_CS_BASE),
              vmx_read(GUEST_CS_LIMIT), vmx_read(GUEST_CS_ACCESS_RIGHT));
    LOG_DEBUG("   DS  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_DS_SELECTOR), vmx_read(GUEST_DS_BASE),
              vmx_read(GUEST_DS_LIMIT), vmx_read(GUEST_DS_ACCESS_RIGHT));
    LOG_DEBUG("   ES  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_ES_SELECTOR), vmx_read(GUEST_ES_BASE),
              vmx_read(GUEST_ES_LIMIT), vmx_read(GUEST_ES_ACCESS_RIGHT));
    LOG_DEBUG("   FS  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_FS_SELECTOR), vmx_read(GUEST_FS_BASE),
              vmx_read(GUEST_FS_LIMIT), vmx_read(GUEST_FS_ACCESS_RIGHT));
    LOG_DEBUG("   GS  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_GS_SELECTOR), vmx_read(GUEST_GS_BASE),
              vmx_read(GUEST_GS_LIMIT), vmx_read(GUEST_GS_ACCESS_RIGHT));
    LOG_DEBUG("   SS  SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_SS_SELECTOR), vmx_read(GUEST_SS_BASE),
              vmx_read(GUEST_SS_LIMIT), vmx_read(GUEST_SS_ACCESS_RIGHT));
    LOG_DEBUG("   LDTR SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n"
              , vmx_read(GUEST_LDTR_SELECTOR), vmx_read(GUEST_LDTR_BASE),
              vmx_read(GUEST_LDTR_LIMIT), vmx_read(GUEST_LDTR_ACCESS_RIGHT));
    LOG_DEBUG("   GDTR SELECTOR:NA BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:NA\n",
              vmx_read(GUEST_GDTR_BASE), vmx_read(GUEST_GDTR_LIMIT));
    LOG_DEBUG("   TR SELECTOR:0x%x BASE:0x%x, LIMIT:0x%x, ACCESS-RIGHT:0x%x\n",
              vmx_read(GUEST_TR_SELECTOR), vmx_read(GUEST_TR_BASE),
              vmx_read(GUEST_TR_LIMIT), vmx_read(GUEST_TR_ACCESS_RIGHT));
    LOG_DEBUG("   IDTR SELECTOR:NA base:0x%x, LIMIT:0x%x, ACCESS-RIGHT:NA\n",
              vmx_read(GUEST_IDTR_BASE), vmx_read(GUEST_IDTR_LIMIT)); 
}
int 
initialize_vmcs(struct vmcs_blob * vm)
{
    // load the vmcs, make the vmcs active and current, but not launched.
    uint32_t * dword_ptr = (uint32_t *)vm->regions.guest_region;
    dword_ptr[0] = get_vmx_revision_id();
    //ASSERT(ERROR_OK == vmx_clear(vm->regions.guest_region));
    ASSERT(ERROR_OK == vmx_load(vm->regions.guest_region));
    initialize_vmcs_ealy_state(vm);
    initialize_vmcs_host_state(vm);
    initialize_vmcs_guest_state(vm);
    initialize_vmcs_execution_control(vm);
    initialize_vmcs_vm_exit_control(vm);
    initialize_vmcs_vm_entry_control(vm);
    initialize_vmcs_ept(vm);
    initialize_vmcs_guest_image(vm);
    // Launch the vm for the first time
    set_current_vm(vm);
    vmx_launch();
    return ERROR_OK;
}

