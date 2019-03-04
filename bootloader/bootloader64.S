/*
 * Copyright (c) 2019 Jie Zheng
 *
 * This is the bootloader which is designed as two stages booting process.
 * and the stage II code is self loaded by stage I code.
 * the features are:
 *      1). Loading the kernel into memory.
 *      2). prepare protected mode context and transfer control to kernel..
 *      3). Optionally encrypt the kernel and prompt a window for user to
 *          deciper it and validate it.
 */
.include "kernel64.inc"

.code16

.equ INITIAL_STACK_END, 0x7C00
.equ SECTOR_BUFFER, 0x200
.equ KERNEL_BASE, 0x100000
.org 0
bootloader_init:
    .byte 0xea
    .word 0x10
    .word 0x07C0

__kernel_loading_sector_index: .word 0x0
__kernel_loading_sector_nr: .word 0x0
__kernel_loading_sector_base: .long 0x0
.org 0x10
bootloader_main:
    cli
    // This is to setup the initial stack, actually, the memory which is lower
    // than 1MB ranging 00001000-0009ebff is free to use, and we choose one
    // segment to load the kernel, the kernel may choose whatever it likes to
    // do kernel stuff.
    // CS,DS, ES, FS, GS = 0x7c0,   SS = 0x0
    xor %ax, %ax
    mov %ax, %ss
    mov $INITIAL_STACK_END, %ax
    mov %ax, %sp
    mov $0x07c0, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    call clear_console
    // Check whether A20 gate is enabled, enable it if the A20 line is disabled
    // the physical space which is greater than 1MB is accessable
    call check_a20gate

    // switch to unreal mode by enter protected mode and switch back, but leave
    // the descriptor cache there untouched afterward.
    // FS, GS = 0x10
    call load_gdt
    movl %cr0, %eax
    orl $0x1, %eax
    movl %eax, %cr0
    movw $0x10, %dx
    movw %dx, %fs
    movw %dx, %gs
    andl $0xfffffffe, %eax
    movl %eax, %cr0

    // Check whether LBA in extended mode is supported in real-mode.
    movb $0x41, %ah
    movw $0x55aa, %bx
    movb $0x80, %dl
    int $0x13
    jc bootloader_end

    // LBA extended mode supported, we load the disk into memory starting from
    // 0x100000.
    movl $BOOTMAIN_SIZE, %ecx
    shrl $0x9, %ecx
    movl $BOOTMAIN_SIZE, %edx
    andl $0x1ff, %edx
    cmpl $0x0, %edx
    je ready_to_load
    incl %ecx
    ready_to_load:
    mov $__kernel_loading_sector_nr, %di
    movw %cx, (%di)
    mov $__kernel_loading_sector_base, %di
    movl $KERNEL_BASE, (%di)
    kernel_copy_loop:
    mov $__kernel_loading_sector_index, %si
    movw (%si), %cx
    mov $__kernel_loading_sector_nr, %di
    cmpw %cx, (%di)
    je kernel_copy_out
    movw %cx, %si
    incl %esi
    mov $__kernel_loading_sector_base, %di
    movl (%di), %edi
    call load_sector
    mov $__kernel_loading_sector_index, %si
    movw (%si), %cx
    incw %cx
    movw %cx, (%si)
    mov $__kernel_loading_sector_base, %di
    movl (%di), %edx
    addl $0x200, %edx
    movl %edx, (%di)
    jmp kernel_copy_loop
    kernel_copy_out:
    //enter protected mode and long jump to kernel entry: 0x100000
    movl %cr0, %eax
    orl $0x1, %eax
    movl %eax, %cr0
    movw $0x10, %dx
    movw %dx, %ds
    movw %dx, %es
    movw %dx, %ss
    movl $KERNEL_BASE, %eax
    ljmpl $0x08, $KERNEL_BASE
            
bootloader_end:
    cli
    hlt
    jmp bootloader_end

/*
 * EDI = the destination memory.
 * ESI = sector index
 * al(out) = 0x0 success, non-0x0, failure
 */
load_sector:
    pushal
    mov $DAP_SECTOR, %edx
    movl %esi, (%edx)
    mov $DAP, %esi
    movb $0x42, %ah
    movb $0x80, %dl
    int $0x13
    // Copy the buffer to the address specified in %EDI
    movl $0x200, %ecx
    copy_kernel_loop:
    movl %edi, %edx
    addl $0x200, %edx
    subl %ecx, %edx
    movl $SECTOR_BUFFER, %ebx
    addl $0x200, %ebx
    subl %ecx, %ebx
    movb %ds:(%ebx), %al
    movb %al, %fs:(%edx)
    loop copy_kernel_loop
    load_sector_out:
    popal
    ret

load_gdt:
    pushal
    movw $gdt_info_offset, %di
    movl $GDT_BASE, (%di)
    addl $0x7c00, (%di)
    movl $GDT_INFO, %eax
    lgdt (%eax)
    popal
    ret
/*
 * This is to check whether the A20 line is enabled, if not, enable it 
 * now ds is 0x07c0, the 0x07c0:0x01fe = 0x7DFE refers to the boot sector
 * identifier xaa55, we read a word from 0xffff:0x7e0e = 0x17dfe, then compare
 * these two words, if they are equal, the gate is disabled. because the 20th
 * address line is ignored at all.
 * XXX: I find another way to deteck whether the A20 gate is enabled. write 
 * different values to address A and address B where A + 0xX00000 = B. note X
 * can be any non-zero hex
 */

check_a20gate:
    pusha
    pushw %es
    movw $0x01fe, %si
    pushw (%si)

    movw $0xffff, %ax
    movw %ax, %es
    movw $0x7e0e, %di
    movw %es:(%di), %ax
    popw %dx

    cmpw %ax, %dx
    je 1f
    movb $0x1, ($a20_enabled)
    jmp 2f
1:
    movb $0x0, ($a20_enabled)
    // Do fast A20 gate activation.
    // see https://wiki.osdev.org/A20_Line for more detail. 
    inb $0x92, %al
    or $0x2, %al
    and $0xfe, %al
    outb %al, $0x92
2:
    popw %es
    popa
    ret

goto_nextline:
    pusha
    movb $0x3, %ah
    movb $0x0, %bh
    int $0x10
    inc %dh
    xor %dl, %dl
    call set_cursor_position
    popa
    ret
/*
 * AH(out) = scancode
 * AL(out) = ascii code if it has one
 */
 wait_keystoke:
    movb $0x0, %ah
    int $0x16
    ret

/*
 * DH = the row index
 * DL = the column index
 */
set_cursor_position:
    pusha
    mov $0x0, %bh
    mov $0x02, %ah
    int $0x10
    popa
    ret

/*
 * DI = the address of the string
 */
print_string:
    pusha
1:
    cmpb $0x0, %ds:(%di)
    je 2f
    movb %ds:(%di), %al
    call print_char
    movb $0x3, %ah
    movb $0x0, %bh
    int $0x10
    inc %dl
    call set_cursor_position
    inc %di
    jmp 1b
2:  
    popa
    ret

/*
 * al = the character
 */

print_char:
    pusha
    movb $0x09, %ah
    movw $0x0004, %bx
    movw $1, %cx
    int $0x10
    popa
    ret


clear_console:
    pusha
    movb $0x0, %ah
    movb $0x2, %al
    int $0x10
    movb $0x0, %dl
    movb $0x0, %dh
    call set_cursor_position
    popa
    ret

.align 8
GDT_INFO:
    gdt_info_size: .word 0x17
    gdt_info_offset: .long 0x0

.align 8
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
.align 4
DAP:
    .byte 0x10
    .byte 0x0
    .word 0x1
    .word 0x7e00
    .word 0x0
DAP_SECTOR: .long 0x0
    .long 0x0
 
// The symbols which reside in the first sectors

// Let the last two bytes be 0x55AA
.org 0x1fe
    .byte 0x55
    .byte 0xaa

.org 0x200
// ============================================================================
// CAVEATS:
// Any content beyond sector one is not referenced in booting stage I
// UNTIL the following content is loaded from disk by stage I code
// Do not put any thing which is needed in earlier stage I below this line
// ============================================================================
