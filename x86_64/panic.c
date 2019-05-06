/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/panic.h>
#include <x86_64/include/per_cpu.h>
#include <lib64/include/logging.h>
#include <x86_64/include/msr.h>
#include <x86_64/include/gdt.h>
#include <x86_64/include/interrupt.h>


extern void * _kernel64_code_start;
extern void * _kernel64_code_end;


static DECLARE_PER_CPU_VARIABLE(struct backtrace_blob, bt_blob);

void
read_auxiliary_state(struct auxiliary_state64 * stat)
{
    stat->cpu = cpuid();
    __asm__ volatile ("movq %%cr0, %%rax;"
                      "movq %%rax, %[CR0];"
                      "movq %%cr2, %%rax;"
                      "movq %%rax, %[CR2];"
                      "movq %%cr3, %%rax;"
                      "movq %%rax, %[CR3];"
                      "movq %%cr4, %%rax;"
                      "movq %%rax, %[CR4];"
                      :[CR0]"=m"(stat->cr0), [CR2]"=m"(stat->cr2),
                       [CR3]"=m"(stat->cr3), [CR4]"=m"(stat->cr4)
                      :
                      :"%rax", "memory");
    uint32_t eax, edx;
    RDMSR(IA32_EFER_MSR, &eax, &edx);
    stat->EFER_MSR = eax | (((uint64_t)edx) << 32);

    RDMSR(FSBASE_MSR, &eax, &edx);
    stat->FS_BASE_MSR = eax | (((uint64_t)edx) << 32);

    RDMSR(GSBASE_MSR, &eax, &edx);
    stat->GS_BASE_MSR = eax | (((uint64_t)edx) << 32); 

    struct gdt_info _gdt;
    __asm__ volatile ("sgdt (%%rax);"
                      :
                      :"a"(&_gdt)
                      :"memory");
    stat->gdt_base = _gdt.offset;
    stat->gdt_size = _gdt.size;

    __asm__ volatile("str %%ax;"
                     :"=a"(stat->TR)
                     :
                     :"memory");

    struct idt_info _idt;
    __asm__ volatile ("sidt (%%rax);"
                      :
                      :"a"(&_idt)
                      :"memory");
    stat->gdt_base = _idt.idt_base_address;
    stat->gdt_size = _idt.idt_limit;

}

void
dump_auxiliary_state(struct auxiliary_state64 * stat)
{
    LOG_INFO_MP_UNSAFE("cpu:%d, idt.base:0x%x idt.size:0x%x\n",
                       stat->cpu, stat->idt_base, stat->idt_size);
    LOG_INFO_MP_UNSAFE("tr:0x%x, gdt.base:0x%x gdt.size:0x%x\n",
                       stat->TR, stat->gdt_base, stat->gdt_size);
    LOG_INFO_MP_UNSAFE("fs.base:0x%x, gs.base:0x%x efer:0x%x\n",
                       stat->FS_BASE_MSR, stat->GS_BASE_MSR, stat->EFER_MSR);
    LOG_INFO_MP_UNSAFE("cr4:0x%x, cr3:0x%x, cr2:0x%x, cr0:0x%x\n",
                       stat->cr4, stat->cr3, stat->cr2, stat->cr0);
}


struct backtrace_blob *
get_backtrace_blob(void)
{
    return THIS_CPU(struct backtrace_blob, bt_blob);
}


void
backtrace(void)
{
    uint64_t rip;
    uint64_t rbp;

    struct backtrace_blob * blob = THIS_CPU(struct backtrace_blob, bt_blob);
    __asm__ volatile ("movq %%rbp, %[RBP];"
                      :[RBP]"=m"(rbp)
                      :
                      :);
    int frame = 0;
    blob->nr_frames = 0;

    for (; frame < MAX_FRAMES_TO_DUMP; frame++) {
        rip = ((uint64_t *)rbp)[1];
        if (rip < (uint64_t)&_kernel64_code_start ||
            rip > (uint64_t)&_kernel64_code_end) {
            break;
        }
        blob->frames[blob->nr_frames] = rip;
        blob->nr_frames += 1;
        LOG_INFO_MP_UNSAFE("    frame #%d <%x>\n", frame, rip);
        rbp = ((uint64_t *)rbp)[0];
    }
}

