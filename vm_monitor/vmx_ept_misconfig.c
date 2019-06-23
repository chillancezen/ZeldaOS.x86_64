/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <memory/include/paging.h>
#include <vm_monitor/include/vmx_ept.h>
#include <vm_monitor/include/vmx_exit.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_instruction_decoding.h>
#include <vm_monitor/include/vmx_mmio.h>


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
    struct mmio_operation * mmio;
    uint64_t guest_addr;
    uint64_t value;
    switch (context.operand_hint)
    {
        case OPERAND_HINT_MR:
            {
                guest_addr = vcpu_get_gp_regiter(vm->vcpu, context.rm_index);
                ASSERT((mmio = search_mmio_callback(guest_addr)));
                ASSERT(mmio->mmio_read);
                vcpu_set_gp_regiter(vm->vcpu, context.reg_index,
                                    mmio->mmio_read(guest_addr,
                                                    context.access_size, exit));
            }
            break;
        case OPERAND_HINT_RM:
            {
                guest_addr = vcpu_get_gp_regiter(vm->vcpu, context.rm_index);
                value = vcpu_get_gp_regiter(vm->vcpu, context.reg_index);
                ASSERT((mmio = search_mmio_callback(guest_addr)));
                ASSERT(mmio->mmio_write);
                mmio->mmio_write(guest_addr, context.access_size, value, exit);
            }
            break;
        case OPERAND_HINT_IM:
            {
                guest_addr = vcpu_get_gp_regiter(vm->vcpu, context.rm_index);
                ASSERT((mmio = search_mmio_callback(guest_addr)));
                ASSERT(mmio->mmio_write);
                mmio->mmio_write(guest_addr, context.access_size,
                                 context.immediate, exit);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break; 
    }
    GOTO_NEXT_INSTRUCTION(exit);
    //PANIC_EXIT(exit); 
    return (uint64_t)exit->vm->vcpu;
}

