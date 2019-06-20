// Copyright (c) 2019 Jie Zheng
// This is the entry point of guest image
// the entry point is going to be executed at 0100:0000, i.e. the begining of
// the second page.
.code16 
.org 0x0
.global guest_entry
.extern guest_kernel_main

.section .guest_kernel_initfn
guest_entry:
    ljmp $0, $0x1000 + 0x10

.org 0x10
    cli
    movw $0x0, %ax
    movw %ax, %ss
    movw $kernel_init_stack, %sp
    call load_gdt
    // Enter protected mode
    movl %cr0, %eax
    bts $0x0, %eax
    movl %eax, %cr0
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ljmp $0x08, $protected_mode

load_gdt:
    pushal
    movw $gdt_info_offset, %di
    movl $GDT_BASE, (%di)
    movl $GDT_INFO, %eax
    lgdt %ds:(%eax)
    popal
    ret

.code32
.section .text
.extern init32_entry
protected_mode:
    call init32_entry
1:
    cli
    hlt
    jmp 1b

.align 8
GDT_INFO:
    gdt_info_size: .word 0x18
    gdt_info_offset: .long 0x0

.align 8
.global GDT_BASE
GDT_BASE:
unsed_descriptor:
    .long 0x0
    .long 0x0
kernel_code_descriptor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0x9a
    .byte 0xcf
    .byte 0x0
kernel_data_descriptor:
    .word 0xffff
    .word 0x0
    .byte 0x0
    .byte 0x92
    .byte 0xcf
    .byte 0x0

.align 8
.section .bss
.space 16*1024 # 16K is supposed enough
.global kernel_init_stack
kernel_init_stack:

