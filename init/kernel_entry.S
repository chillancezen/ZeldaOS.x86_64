/*
 * Copyright (c) 2019 Jie Zheng
 *
 */

#include "zelda64_config.h"
#include <x86_64/include/msr.h>
#include <x86_64/include/lapic.h>
.code32
.extern kernel_main
.extern kernel_ap_main
.extern apic_supported
.extern msr_supported
.extern check_bootstrap_processor
.extern broadcast_init_ipi
.extern broadcast_startup_ipi
.extern enable_apic
.extern gdt_entries

.equ IA32_EFER, 0xC0000080
.equ PAE_FLAG, 0x20
.equ PSE_FLAG, 0x10
.equ LONGMODE_ENABLE, 0x100
.equ PAGE_PRESENT, 0x1
.equ PAGE_WRITTABLE, 0x2
.equ PAGE_USER, 0x4
.equ PAGE_1GB_SIZE, 0x80
.equ PAGE_2MB_SIZE, 0x80
.equ CR0_PAGING_ENABLE, 0x80000000
.section .kernel_initfn
.global kernel_entry
kernel_entry:
    // identifed as the BSP
    movl $APIC_ID_REGISTER, %edi
    movl (%edi), %eax
    shrl $24, %eax
    incl %eax
    movl $STACK_SIZE_PER_CPU, %ecx
    mull %ecx
    addl $SMP_STACK, %eax
    movl %eax, %esp

    // Enter IA-32e mode
    movl %cr4, %edx
    orl $PAE_FLAG, %edx        
    movl %edx, %cr4
    // Install the earlier page table
    call construct_early_page_table
    movl $EARLY_PML4_BASE, %eax
    movl %eax, %cr3
    call enable_longmode
    // Activate long mode via enable paging
    movl %cr0, %eax
    orl $CR0_PAGING_ENABLE, %eax
    movl %eax, %cr0
    call load_gdt64
    //call kernel main
    jmp kernel_main
    cli
    hlt

.org 0x200
kernel_ap_entry:
    cli
    movw $0x10, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %ss
    movw %dx, %fs
    movw %dx, %gs
    // identify the lapic id and select the stack for it
    movl $APIC_ID_REGISTER, %edi
    movl (%edi), %eax
    shrl $24, %eax
    incl %eax
    movl $STACK_SIZE_PER_CPU, %ecx
    mull %ecx
    addl $SMP_STACK, %eax
    movl %eax, %esp
    // enable IA-32e mode by following the steps in BSP
    movl %cr4, %edx
    orl $PAE_FLAG, %edx
    movl %edx, %cr4

    movl $EARLY_PML4_BASE, %eax
    movl %eax, %cr3
    call enable_longmode

    movl %cr0, %eax
    orl $CR0_PAGING_ENABLE, %eax
    movl %eax, %cr0
    call load_gdt64
    
    // switch to kernel ap 64-bit code
    jmp kernel_ap_main
    cli
    hlt

load_gdt64:
    movl $GDT64_offset, %edi
    movl $gdt_entries, (%edi)
    movl $GDT64_pointer, %eax
    lgdt (%eax)
    movw $0x10, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %ss
    ljmp $0x08, $1f
1:
    ret
GDT64_pointer:
    .word 0x30
GDT64_offset: .long 0x0


// This is to write IA32_EFER.LME to enable long mode
enable_longmode:
    pushal
    xorl %eax, %eax
    xorl %edx, %edx
    movl $IA32_EFER_MSR, %ecx
    rdmsr
    orl $IA32_EFER_MSR_LME, %eax
    wrmsr
    popal
    ret

//@param None
//@return None
// This is to construct the early paging table for Long Mode
// as the most large PAGE size is 1G, we need at least one PDPT array to map the
// first 4G memory
construct_early_page_table:
    pushal
    // clear the PML4 4K page
    movl $EARLY_PML4_BASE, %edi
    movl $1024, %ecx
1:
    movl $0x0, (%edi)
    addl $4, %edi
    loop 1b
    // install the 0th paging entry
    // Let %eax hold the low 32bit dword and %edx hold the high 32bit dword
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $EARLY_PDPT_BASE, %eax
    movl $EARLY_PML4_BASE, %edi
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    // clear the whole PDPTE
    movl $EARLY_PDPT_BASE, %edi
    movl $1024, %ecx
2:
    movl $0x0, (%edi)
    andl $0x4, %edi
    loop 2b
    // install the PDPTE in another 4K page
    // here we only map the lowest 1G memory, which means we have only 1 PDPTE
    movl $EARLY_PDPT_BASE, %edi
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $EARLY_PD_BASE, %eax
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    // clear the page directory
    movl $EARLY_PD_BASE, %edi
    movl $1024, %ecx
    movl $0x0, %ebx
3:
    movl $0x0, (%edi)
    addl $4, %edi
    loop 3b
    // fill the 512 2B page mapping in EARLY_PD_BASE
    movl $EARLY_PD_BASE, %edi
    movl $512, %ecx
    movl $0x0, %ebx
4:
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $PAGE_2MB_SIZE, %eax
    orl %ebx, %eax
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    addl $8, %edi
    addl $0x200000, %ebx
    loop 4b
    popal
    ret
   

//@param None
//@return None
MESSAGE_ENTER_BSP_ROUTINES: .string "[INFO] Enter BSP Routine"
bsp_routines:
    call check_bootstrap_processor
    cmpl $0x0, %eax
    je bsp_routines_out
    movl $MESSAGE_ENTER_BSP_ROUTINES, %esi
    call print_message
    call enable_apic
    // Send INIT-IPI to all other processors.
    call broadcast_init_ipi
    movl $100000000, %ecx
    rep nop
    // Send StartUp-IPI to all other processors.
    call broadcast_startup_ipi
    call broadcast_startup_ipi
bsp_routines_out:
    ret

//@param None
//@return None
MESSAGE_ENTER_AP_ROUTINES: .string "[INFO] enter AP routine"
ap_routines:
    call check_bootstrap_processor
    cmpl $0x0, %eax
    jne ap_routines_out
    movl $MESSAGE_ENTER_AP_ROUTINES, %esi
    call print_message
ap_routines_out:
    ret
//@param None
//@return None
MESSAGE_BSP_FOUND: .string "[INFO] Bootstrap Processor Found"
check_bsp:
    call check_bootstrap_processor
    cmpl $0x0, %eax
    je check_bsp_out
    movl $MESSAGE_BSP_FOUND, %esi
    call print_message
check_bsp_out:
    ret


//@param None
//@return None
.global __apic_supported
__apic_supported: .long 0x0
MESSAGE_NO_APIC: .string "[ERROR] CPU does not support APIC feature"
MESSAGE_HAS_APIC: .string "[INFO] CPUID.APIC Detected"
check_apic:
    pushal
    call apic_supported
    movl $__apic_supported, %edi
    cmpl $0x0, %eax
    setnz (%edi)
    cmpb $0x0, (%edi)
    jnz check_apic_out
    movl $MESSAGE_NO_APIC, %esi
    call print_message
    call kernel_halt
check_apic_out:
    movl $MESSAGE_HAS_APIC, %esi
    call print_message
    popal
    ret

//@param None
//@return None
.global __msr_supported
__msr_supported: .long 0x0
MESSAGE_NO_MSR: .string "[ERROR] CPU does not support MSR feature"
MESSAGE_HAS_MSR: .string "[INFO] CPUID.MSR detected"
check_msr:
    pushal
    call msr_supported
    movl $__msr_supported, %edi
    cmpl $0x0, %eax
    setnz (%edi)
    cmpb $0x0, (%edi)
    jnz check_msr_out
    movl $MESSAGE_NO_MSR, %esi
    call print_message
    call kernel_halt
check_msr_out:
    movl $MESSAGE_HAS_MSR, %esi
    call print_message
    popal
    ret

//@param %ESI : the address of the string
//@return None
__row_index: .long 0x0
print_message:
    pushal
    movl $__row_index, %edi
    movl (%edi), %ebx
    call print_string
    movl $__row_index, %edi
    movl (%edi), %edx
    incl %edx
    cmpl $24, %edx
    jnz print_message_out
    movl $0x0, %edx
print_message_out:
    movl %edx, (%edi)
    popal
    ret

//@param %EBX : the row index
//@param %ESI : the address of the string
//@return None
print_string:
    pushal
    movl $0xb8000, %edi
    movl %ebx, %eax
    movl $160, %ecx
    mull %ecx
    addl %eax, %edi
print_string_loop:
    cmpb $0x0, (%esi)
    jz print_string_end
    movb (%esi), %dl
    movb %dl, (%edi)
    incl %esi
    addl $0x2, %edi
    jmp print_string_loop
print_string_end:
    popal
    ret

// put the cpu into halt state, never wake it again.
kernel_halt_message: .string "[WARNNING MESSAGE] Kernel ZeldaOS.x86_64 Halts"
kernel_halt:
    movl $24, %ebx
    movl $kernel_halt_message, %esi
    call print_string
1:
    cli
    hlt
    jmp 1b
    ret

// The STACK space for all processors
.section .bss
.align 32
.global SMP_STACK
.global SMP_STACK_END
SMP_STACK:
    .space STACK_SIZE_PER_CPU * MAX_NR_CPUS
SMP_STACK_END:

