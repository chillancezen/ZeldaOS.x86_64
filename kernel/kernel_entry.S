// Copyright (c) 2019 Jie Zheng

.code32

.section .kernel_initfn
.global kernel_entry
kernel_entry:
    cli
    hlt 

