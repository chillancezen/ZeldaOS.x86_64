/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_vcpu_state.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_exit.h>

#define MAX_NR_VMEXIT_REASONS 65

typedef uint64_t vmexit_sub_handler(struct vmexit_info * exit);
static vmexit_sub_handler * sub_handlers[MAX_NR_VMEXIT_REASONS];

static uint64_t
io_instruction_exit_sub_handler(struct vmexit_info * exit)
{

    return (uint64_t)exit->vm->vcpu;
}

uint64_t
vm_exit_handler_entry(struct guest_cpu_state * vcpu)
{
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
    ASSERT(!(info.basic_reason & (1 << 31)));
    {
        int reason_index = info.basic_reason & 0xffff;
        ASSERT(reason_index < MAX_NR_VMEXIT_REASONS);
        if (sub_handlers[reason_index]) {
            rsp = sub_handlers[reason_index](&info);
        }
    }
    dump_vm(vm);
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
    _(BASIC_VMEXIT_REASON_IO_INSTRUCTION, io_instruction_exit_sub_handler);
#undef _
}
