; boot/multiboot.asm
; Multiboot header untuk x86

MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ (1 << 0) | (1 << 1) | (1 << 2)   ; minta meminfo + boot device + memory map
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
