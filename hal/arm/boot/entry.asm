.global _start

_start:
    /* Set stack pointer */
    ldr x0, =stack_top
    mov sp, x0

    /* Clear BSS */
    ldr x0, =_bss_start
    ldr x1, =_bss_end
    mov x2, #0
1:
    cmp x0, x1
    b.ge 2f
    str x2, [x0], #8
    b 1b
2:
    /* Jump ke kmain */
    bl kmain

.hang:
    wfe
    b .hang