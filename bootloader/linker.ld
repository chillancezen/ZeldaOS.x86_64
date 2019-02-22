ENTRY(stage2_main)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    . = 0x7e00;
    .text :
    {
        *(.text*)
        *(.rodata)
    }
    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
    /DISCARD/ : { *(.fini_array*) *(.comment) }
}
