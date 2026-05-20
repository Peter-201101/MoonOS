ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    _kernel_start = .;

    .multiboot :
    {
        *(.multiboot)
    }

    .text ALIGN(4096) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4096) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4096) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4096) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    _kernel_end = .;
    _end = .;
}
