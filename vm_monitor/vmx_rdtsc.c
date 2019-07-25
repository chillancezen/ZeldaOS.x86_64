/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <vm_monitor/include/vmx_rdtsc.h>
#include <vm_monitor/include/vmx_exit.h>
#include <x86_64/include/misc.h>

uint64_t
rdtsc_exit_sub_handler(struct vmexit_info * exit)
{
    uint64_t curr = rdtsc();
    exit->vm->vcpu->rdx = (curr >> 32) & 0xffffffff;
    exit->vm->vcpu->rax = curr & 0xffffffff;
    exit->vm->vcpu->rcx = CPU_FREQUENCY;
    GOTO_NEXT_INSTRUCTION(exit);
    return (uint64_t)exit->vm->vcpu;
}
