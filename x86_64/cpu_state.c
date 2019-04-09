/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <x86_64/include/cpu_state.h>
#include <lib64/include/printk.h>


void
dump_cpu_state(struct cpu_state64 * cpu, int mp_safe)
{
    if (mp_safe) {
        printk("[DUMP] cpu state at frame:0x%x\n", cpu);
        printk("   ss:0x%x rsp:0x%x rflags:0x%x\n", cpu->ss, cpu->rsp,
               cpu->rflags);
        printk("   cs:0x%x rip:0x%x vector:0x%x(%d) error_code:0x%x\n",
               cpu->cs, cpu->rip, cpu->vector, cpu->vector, cpu->error_code);
        printk("   rax:0x%x, rbx:0x%x, rcx:0x%x, rdx:0x%x\n",
               cpu->rax, cpu->rbx, cpu->rcx, cpu->rdx);
        printk("   rsi:0x%x, rdi:0x%x, rbp:0x%x\n",
               cpu->rsi, cpu->rdi, cpu->rbp);
        printk("   r8:0x%x r9:0x%x r10:0x%x r11:0x%x\n",
               cpu->r8, cpu->r9, cpu->r10, cpu->r11);
        printk("   r12:0x%x, r13:0x%x, r14:0x%x, r15:0x%x\n",
               cpu->r12, cpu->r13, cpu->r14, cpu->r15);
        printk("   ds:0x%x, es:0x%x, fs:0x%x, gs:0x%x\n",
               cpu->ds, cpu->es, cpu->fs, cpu->gs);
    } else {
        printk_mp_unsafe("[DUMP] cpu state at frame:0x%x\n", cpu);
        printk_mp_unsafe("   ss:0x%x rsp:0x%x rflags:0x%x\n", cpu->ss, cpu->rsp,
                         cpu->rflags);
        printk_mp_unsafe("   cs:0x%x rip:0x%x vector:0x%x(%d) "
                         "error_code:0x%x\n",
                         cpu->cs, cpu->rip, cpu->vector, cpu->vector,
                         cpu->error_code);
        printk_mp_unsafe("   rax:0x%x, rbx:0x%x, rcx:0x%x, rdx:0x%x\n",
                         cpu->rax, cpu->rbx, cpu->rcx, cpu->rdx);
        printk_mp_unsafe("   rsi:0x%x, rdi:0x%x, rbp:0x%x\n",
                         cpu->rsi, cpu->rdi, cpu->rbp);
        printk_mp_unsafe("   r8:0x%x r9:0x%x r10:0x%x r11:0x%x\n",
                         cpu->r8, cpu->r9, cpu->r10, cpu->r11);
        printk_mp_unsafe("   r12:0x%x, r13:0x%x, r14:0x%x, r15:0x%x\n",
                         cpu->r12, cpu->r13, cpu->r14, cpu->r15);
        printk_mp_unsafe("   ds:0x%x, es:0x%x, fs:0x%x, gs:0x%x\n",
                         cpu->ds, cpu->es, cpu->fs, cpu->gs);
    }
}
