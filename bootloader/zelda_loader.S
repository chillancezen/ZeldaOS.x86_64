/*
 * Copyright (c) 2019 Jie Zheng
 *
 * This is the bootloader which is designed as two stages booting process.
 * and the stage II code is self loaded by stage I code.
 * the features are:
 *      1). Loading the kernel into memory.
 *      2). Stay in real address mode and transfer control to kernel..
 *      3). Optionally encrypt the kernel and prompt a window for user to
 *          deciper it and validate it.
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
    call goto_nextline
    call goto_nextline
    // Check whether A20 gate is enabled, enable it if the A20 line is disabled
    call check_a20gate

    // Load the kernel into memory
    mov $loading_message, %di
    call printstring
    call goto_nextline

bootloader_end:
    cli
    hlt
    jmp bootloader_end

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
    mov $a20gate_enabled, %di
    call printstring
    call goto_nextline
    jmp 2f
1:
    mov $a20gare_disabled, %di
    call printstring
    call goto_nextline
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
a20gate_enabled : .string "[Step 1] Checking A20 gate: enabled"
a20gare_disabled: .string "[Step 1] Checking A20 gate: disabled"
loading_message: .string "[Step 2] Loading left sectors"
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


