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
    ljmp $0x100, $0x10

.org 0x10
    movw $1, %dx
    inb %dx, %al
1:
    cli
    movw $0x1235, %ax
    hlt
    jmp 1b

