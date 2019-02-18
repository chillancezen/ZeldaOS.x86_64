/*
 * Copyright (c) 2019 Jie Zheng
 *
 * This is the bootloader which is designed as two stages booting process.
 * and the stage II code is self loaded by stage I code.
 * the features are:
 *      1). Loading the kernel into memory.
 *      2). Stay in real address mode and transfer control to kernel..
 *      3). Play an optional game before enter OS/kenrel.
 */

.code16

.equ INITIAL_STACK_END, 0x7C00

.org 0
bootloader_init:
    .byte 0xea
    .word 0x10
    .word 0x07C0

.org 0x10
bootloader_main:
    cli
    // This is to setup the initial stack, actually, the memory which is lower
    // than 1MB ranging 00001000-0009ebff is free to use, and we choose one
    // segment to load the kernel, the kernel may choose whatever it likes to
    // do kernel stuff.
    xor %ax, %ax
    mov %ax, %ss
    mov $INITIAL_STACK_END, %ax
    mov %ax, %sp
    mov $0x07c0, %ax
    mov %ax, %ds

    // Print some welcome message here.
    call clear_console
    mov $copyright_message, %di
    call printstring
    movb $0x1, %dh
    movb $0x0, %dl
    call set_cursor_position
    mov $hint_message, %di
    call printstring
    // wait for key stroke and do left work.
    call wait_keystoke
    movb $0x3, %ah
    movb $0x0, %bh
    int $0x10
    inc %dh
    xor %dl, %dl
    call set_cursor_position

bootloader_end:
    cli
    hlt
    jmp bootloader_end

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
printstring:
    pusha
1:
    cmpb $0x0, (%di)
    je 2f
    movb (%di), %al
    call printchar
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

printchar:
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
// The symbols which reside in the first sectors
copyright_message: .string "Copyright (c) 2018-2019 Jie Zheng [at] VMware"
hint_message : .string "Press any key to boot ZeldaOS.x86_64 ..."

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
    
