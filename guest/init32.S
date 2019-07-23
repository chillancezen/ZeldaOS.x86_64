#Copyright (c) 2019 Jie Zheng
.section .text
.code32

.extern  guest_kernel_main
.extern initial_gdt_info
.global init32_entry
init32_entry:
    // Enable PAE
    movl %cr4, %eax
    btsl $0x5, %eax
    movl %eax, %cr4

    call construct_early_page_table
    movl $pml4_base, %eax
    movl %eax, %cr3

    call enable_longmode

    movl %cr0, %eax
    btsl $31, %eax
    movl %eax, %cr0
    // Now CPU enters compatible long mode, not genuine 64-bit long mode
    movl $initial_gdt_info, %eax
    lgdt (%eax)
    movw $0x10, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %ss
    ljmp $0x08, $1f
1:
    call  guest_kernel_main
    sti
2:
    hlt
    jmp 2b


.equ IA32_EFER_MSR, 0xc0000080
.equ IA32_EFER_MSR_SCE, 0x1
.equ IA32_EFER_MSR_LME, 0x100
.equ IA32_EFER_MSR_LMA, 0x400
.equ IA32_EFER_MSR_NXE, 0x800
enable_longmode:
    xorl %eax, %eax
    xorl %edx, %edx
    movl $IA32_EFER_MSR, %ecx
    rdmsr
    orl $IA32_EFER_MSR_LME, %eax
    wrmsr
    ret

.equ PAGE_PRESENT, 0x1
.equ PAGE_WRITTABLE, 0x2
.equ PAGE_USER, 0x4
.equ PAGE_1GB_SIZE, 0x80
.equ PAGE_2MB_SIZE, 0x80
construct_early_page_table:
    movl $pml4_base, %edi
    movl $1024 * 3, %ecx
1:
    movl $0x0, (%edi)
    addl $4, %edi
    loop 1b
    // Install Level-4 entry
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $pdpt_base, %eax
    movl $pml4_base, %edi
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    // Install Level-3 entry
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $pdt_base, %eax
    movl $pdpt_base, %edi
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    // Install Level-2 entry: 512 2MB-pages.
    movl $pdt_base, %edi
    movl $512, %ecx
    xorl %ebx, %ebx
2:
    xorl %eax, %eax
    orl $PAGE_PRESENT, %eax
    orl $PAGE_WRITTABLE, %eax
    orl $PAGE_2MB_SIZE, %eax
    orl %ebx, %eax
    movl %eax, (%edi)
    movl $0x0, 4(%edi)
    addl $8, %edi
    addl $0x200000, %ebx
    loop 2b
    ret

.section .bss
.global pml4_base
.global pdpt_base
.global pdt_base
.align 4096
pml4_base:
.space 4096
pdpt_base:
.space 4096
pdt_base:
.space 4096
