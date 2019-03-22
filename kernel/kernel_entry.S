/*
 * Copyright (c) 2019 Jie Zheng
 *
 */

#include "zelda64_config.h"
.code32
.extern kernel_main
.extern apic_supported
.extern msr_supported
.extern check_bootstrap_processor
.extern broadcast_init_ipi
.extern broadcast_startup_ipi
.extern enable_apic

.section .kernel_initfn
.global kernel_entry
kernel_entry:
    movl $BSP_STACK, %esp
    call check_apic
    call check_msr
    call check_bsp
    call bsp_routines
    call ap_routines 
    call kernel_halt

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

.align 8
.section .bss
    .space 4 * 1024 * 1024
.global BSP_STACK
BSP_STACK:
