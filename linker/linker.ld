/* ==========================================
   MoonOS - linker.ld
   Ngatur layout memori kernel di x86_64
   ========================================== */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS {
    . = 1M;
    .text ALIGN(4K) : {
        KEEP(*(.multiboot2))
        *(.text*)
    }
    .rodata ALIGN(4K) : { *(.rodata*) }
    .data ALIGN(4K) : { *(.data*) }
    .bss ALIGN(4K) : { 
        *(COMMON) 
        *(.bss*) 
    }
    kernel_end = .;
}