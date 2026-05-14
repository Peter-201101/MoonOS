.global _start
.global _bss_start
.global _bss_end

_start:
    /* Hanya core 0 yang jalan, sisanya tidur */
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbnz x0, .hang

    /* Setup stack */
    ldr x0, =stack_top
    mov sp, x0

    /* Clear BSS */
    ldr x0, =_bss_start
    ldr x1, =_bss_end
    mov x2, #0
.clear_bss:
    cmp x0, x1
    b.ge .done_bss
    str x2, [x0], #8
    b .clear_bss
.done_bss:

    /* Jump ke kmain */
    bl kmain

.hang:
    wfe
    b .hang

/* Exception vectors — minimal */
.balign 0x800
.global vectors
vectors:
    b .hang   /* Current EL, SP0, Sync */
    .balign 0x80
    b .hang   /* Current EL, SP0, IRQ  */
    .balign 0x80
    b .hang
    .balign 0x80
    b .hang
    .balign 0x80
    b .hang   /* Current EL, SPx, Sync */
    .balign 0x80
    b arm_irq_handler  /* Current EL, SPx, IRQ */
    .balign 0x80
    b .hang
    .balign 0x80
    b .hang

/* Kernel stack */
.section .bss
.balign 16
.space 16384
stack_top:

