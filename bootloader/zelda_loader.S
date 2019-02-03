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
    
    mov $0x10, %dh
    mov $0x20, %dl
    call set_cursor_position
    
bootloader_end:
    cli
    hlt
    jmp bootloader_end


/*
 * AH = 0x2
 * DH = the row index
 * DL = the column index
 */
set_cursor_position:
    pushw %bx
    pushw %ax
    mov $0x0, %bh
    mov $0x02, %ah
    int $0x10
    popw %ax
    popw %bx
    ret


__row_index: .byte 0x0
__col_index: .byte 0x0
clear_console:
     
        
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
    
welcome_message : .string "Welcome come to ZeldaOS.x86_64 Bootloader"
