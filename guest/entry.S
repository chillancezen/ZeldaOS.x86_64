// Copyright (c) 2019 Jie Zheng
// This is the entry point of guest image
// the entry point is going to be executed at 0100:0000, i.e. the begining of
// the second page.
.code16 
.org 0

.global guest_entry
.extern guest_kernel_main

.section .guest_kernel_initfn
guest_entry:
1:
    cli
    hlt
    jmp 1b

