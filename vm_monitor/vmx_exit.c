/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_vcpu_state.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_exit.h>
#include <vm_monitor/include/vmx_event.h>
#define MAX_NR_VMEXIT_REASONS 65

static vmexit_sub_handler * sub_handlers[MAX_NR_VMEXIT_REASONS];


static uint64_t
controll_register_access_exit_sub_handler(struct vmexit_info * exit)
{
    struct guest_cpu_state * vcpu = exit->vm->vcpu;
    // See Table 27-3. Exit Qualification for Control-Register Accesses
    uint64_t qualification = exit->exit_qualification;
    uint8_t  ctrl_reg_index = qualification & 0xf;
    uint8_t access_type = (qualification >> 4) & 0x3;
#define MOV_TO_CR 0x0
#define MOV_FROM_CR 0x1
    uint8_t gp_reg = (qualification >> 8) & 0xf;
    if (ctrl_reg_index == 3) {
        if (access_type == MOV_TO_CR) {
            // XXX: here we say only %RAX is legal to contain the operand for
            // the sake of simplicity
            ASSERT(gp_reg == 0);
            uint64_t cr3_content = vcpu->rax;
            vmx_write(GUEST_CR3, cr3_content);
            LOG_DEBUG("vm:0x%x(vpid:%d) cr3 set to:0x%x\n", exit->vm,
                      exit->vm->vpid, cr3_content);
            GOTO_NEXT_INSTRUCTION(exit);
        } else {
            PANIC_EXIT(exit);
        }
    } else {
        PANIC_EXIT(exit);
    }
    return (uint64_t)exit->vm->vcpu;
}

static uint64_t
rdmsr_exit_sub_handler(struct vmexit_info * exit)
{
    struct guest_cpu_state * vcpu = exit->vm->vcpu;
    switch (vcpu->rcx)
    {
        case 0xc0000080: // IA32_EFER
            {
                uint64_t guest_efer = vmx_read(GUEST_IA32_EFER);
                vcpu->rax = (uint32_t)guest_efer;
                vcpu->rdx = (uint32_t)(guest_efer >> 32);
                GOTO_NEXT_INSTRUCTION(exit);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    return (uint64_t)exit->vm->vcpu;
}

static uint64_t
wrmsr_exit_sub_handler(struct vmexit_info * exit)
{
    struct guest_cpu_state * vcpu = exit->vm->vcpu;
    switch (vcpu->rcx)
    {
        case 0xc0000080: // IA32_EFER
        {
            uint64_t guest_efer;
            guest_efer = vcpu->rax & 0xffffffff;
            guest_efer |= (vcpu->rdx << 32) & 0xffffffff;
            vmx_write(GUEST_IA32_EFER, guest_efer);
            LOG_DEBUG("vm:0x%x(vpid:%d) msr:0x%x set to:0x%x\n",
                      exit->vm, exit->vm->vpid, vcpu->rcx, guest_efer);
            GOTO_NEXT_INSTRUCTION(exit);
        }
        break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    return (uint64_t)exit->vm->vcpu;
}

static uint64_t
halt_exit_sub_handler(struct vmexit_info * exit)
{
    //inject_external_event(exit, 32);
    GOTO_NEXT_INSTRUCTION(exit);
    //PANIC_EXIT(exit);
    return (uint64_t)exit->vm->vcpu;
}

uint64_t
vm_exit_handler_entry(struct guest_cpu_state * vcpu)
{
    {
        // Adjust the IA32_EFER.LMA bit in vmcs, and later reloaded into guest
        // This is weird with intel VT-x vmx. please See 27.2 for more.
        uint64_t vm_entry_ctls = vmx_read(CTLS_VM_ENTRY);
        uint64_t guest_efer = vmx_read(GUEST_IA32_EFER);
        if (vm_entry_ctls & (1 << 9)) {
            vmx_write(GUEST_IA32_EFER, guest_efer | (1 << 10));
        } else  {
            vmx_write(GUEST_IA32_EFER, guest_efer & ~(1 << 10));
        }
    }
    uint64_t rsp = (uint64_t)vcpu;
    struct vmcs_blob * vm = get_current_vm();
    ASSERT(vm);
    vm->vcpu = vcpu;
    
    struct vmexit_info info;
    info.vm = vm;
    info.basic_reason = vmx_read(VMEXIT_REASON);
    info.exit_qualification = vmx_read(VMEXIT_QUALIFICATION);
    info.guest_linear_addr = vmx_read(VMEXIT_GUEST_LINEAR_ADDR);
    info.guest_physical_addr = vmx_read(VMEXIT_GUEST_PHYSICAL_ADDR);
    info.instruction_length = vmx_read(VMEXIT_INSTRUCTION_LENGTH);
    info.instruction_info = vmx_read(VMEXIT_INSTRUCTION_INFO);
    if (info.basic_reason & (1 << 31)) {
        LOG_TRIVIA("exit qualification:0x%x\n", info.exit_qualification);
        PANIC_EXIT(&info);
    }
    {
        int reason_index = info.basic_reason & 0xffff;
        ASSERT(reason_index < MAX_NR_VMEXIT_REASONS);
        LOG_TRIVIA("exit {basic:%d, qualification:0x%x, instruction-len:%d "
                   "instruction-info:0x%x guest-linear-addr:0x%x "
                   "guest-phy-addr:0x%x}\n",
                   reason_index, info.exit_qualification,
                   info.instruction_length,
                   info.instruction_info,
                   info.guest_linear_addr,
                   info.guest_physical_addr);
        if (sub_handlers[reason_index]) {
            rsp = sub_handlers[reason_index](&info);
        } else {
            PANIC_EXIT(&info);
        }
    }
    return rsp;
}

__attribute__((constructor)) static void
vm_exit_handler_init(void)
{
    memset(sub_handlers, 0x0, sizeof(sub_handlers));
#define _(reason, handler) {                                                   \
    ASSERT((reason) >= 0 && (reason) < MAX_NR_VMEXIT_REASONS);                 \
    sub_handlers[(reason)] = (handler);                                        \
}
    _(BASIC_VMEXIT_REASON_EXTERNAL_INTERRUPT, external_interrupt_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_IO_INSTRUCTION, io_instruction_exit_sub_handler);
    _(BASIC_VMEXIT_CONTROL_REGISTER_ACCESS, controll_register_access_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_HLT, halt_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_RDMSR, rdmsr_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_WRMSR, wrmsr_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_EPT_MISCONFIG, ept_misconfig_exit_sub_handler);
    _(BASIC_VMEXIT_REASON_INTERRUPT_WINDOW, interrupt_window_exit_sub_handler);
#undef _
}
