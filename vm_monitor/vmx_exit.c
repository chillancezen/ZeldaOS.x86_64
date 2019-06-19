/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_vmcs.h>
#include <vm_monitor/include/vmx_vcpu_state.h>
#include <lib64/include/logging.h>


uint64_t
vm_exit_handler_entry(struct guest_cpu_state * vcpu)
{
    uint64_t rsp = (uint64_t)vcpu;
    struct vmcs_blob * vm = get_current_vm();
    LOG_TRIVIA("vm exit for vm:0x%x(vpid:%d)\n", vm, vm->vpid);

    uint64_t exit_reason = vmx_read(VMEXIT_REASON);
    // It must not be a vm-entry failure.
    ASSERT(!(exit_reason & (1 << 31)));

    LOG_TRIVIA("EXIT:%x\n", exit_reason);
    
    return rsp;
}
