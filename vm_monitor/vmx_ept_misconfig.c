/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <memory/include/paging.h>
#include <vm_monitor/include/vmx_ept.h>
#include <vm_monitor/include/vmx_exit.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_instruction_decoding.h>

uint64_t
ept_misconfig_exit_sub_handler(struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    uint64_t guest_instruction_addr =
        guestpa_to_hostpa(vm->regions.ept_pml4_base, vmx_read(GUEST_RIP));
    ASSERT(pa(guest_instruction_addr) == guest_instruction_addr);
    
    struct instruction_context context;
    ASSERT(decode_x86_64_instruction((uint8_t *)guest_instruction_addr,
                                     &context) == ERROR_OK);
    exit->instruction_length = context.instruction_length;
    
    GOTO_NEXT_INSTRUCTION(exit);
    //PANIC_EXIT(exit); 
    return (uint64_t)exit->vm->vcpu;
}



