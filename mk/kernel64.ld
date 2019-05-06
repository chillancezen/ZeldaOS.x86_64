ENTRY(kernel_entry)
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
    . = 0x0100000;

    .text BLOCK(4K) : ALIGN(4K)
    {
        _kernel64_code_start = .;
        *(.kernel_initfn*)
        *(.text*)
        _kernel64_code_end = .;
        *(.rodata*)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        _kernel64_constructor_start = .;
        KEEP(*( .init_array ));
        KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
        _kernel64_constructor_end = .;
        *(.data*)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss*)
    }
    . = ALIGN(4096);
    /DISCARD/ :
    {
        *(.fini_array*)
        *(.comment*)
    }
    . = ALIGN(4096);
    _kernel64_image_end = .;
}
