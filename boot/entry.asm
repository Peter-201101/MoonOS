BITS 32

section .note.GNU-stack noalloc noexec nowrite progbits

section .bss
align 16
stack_bottom:
    resb 16384      ; 16KB
stack_top:

section .text
global _start
extern kmain

_start:
    mov esp, stack_top
    and esp, 0xFFFFFFF0
    push 0
    popf
    push ebx    ; multiboot_info_t*
    push eax    ; magic
    call kmain
.hang:
    cli
    hlt
    jmp .hang