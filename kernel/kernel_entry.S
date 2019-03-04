// Copyright (c) 2019 Jie Zheng

.code32

.section .kernel_initfn
.global kernel_entry
kernel_entry:
    mov $0xb8000, %edi
    movb $'H', (%edi)
    addl $0x2, %edi
    movb $'L', (%edi)
    cli
    hlt 

.org 0x200

cute : .string "Hello KErnel"
