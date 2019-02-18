.set MB_MAGIC,      0x1badb002
.set MB_FLAGS,      (1<<0 | 1<<1)
.set MB_CHECKSUM,   -(MB_MAGIC + MB_FLAGS)


.section .multiboot
mb_start:
    .int MB_MAGIC
    .int MB_FLAGS
    .int MB_CHECKSUM
mb_end:

.section .text
.global __zelda_boot_entry
.extern kernel32_init

__zelda_boot_entry:
    cli
    mov $kernel_stack, %esp
    push %eax
    push %ebx
    # here we hardcode the 64bit code as 0x800000
    # we must make sure it starts at exactly 8MB address.
    call kernel32_init

__zelda_boot_hlt:
    cli
    hlt
    jmp __zelda_boot_hlt
.code64
    movq $0, %rax
    andq $0, %rax

.section .bss
.space 4*1024*1024
kernel_stack:



.section .raw_kernel64
.incbin "Zelda64.code64"
