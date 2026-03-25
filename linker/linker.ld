/* ==========================================
   MoonOS - linker.ld
   Ngatur layout memori kernel di x86_64
   ========================================== */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS {
    /* Kernel dimuat di 1MB */
    . = 1M;

    /* Code section */
    .text ALIGN(4K) : {
        KEEP(*(.multiboot2))    /* Multiboot header harus di awal */
        *(.text*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    /* Data yang bisa ditulis */
    .data ALIGN(4K) : {
        *(.data*)
    }

    /* BSS - variabel global yang belum diinisialisasi */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss*)
    }

    /* Tandai akhir kernel — dipakai PMM buat tahu batas kernel */
    kernel_end = .;
}
