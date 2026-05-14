BITS 32

section .note.GNU-stack noalloc noexec nowrite progbits

; External C handlers
extern isr_handler
extern irq_handler
extern syscall_dispatch

; Macro: ISR tanpa error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push dword 0        ; dummy error code
    push dword %1       ; interrupt number
    jmp isr_common
%endmacro

; Macro: ISR dengan error code (CPU push sendiri)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push dword %1       ; interrupt number
    jmp isr_common
%endmacro

; Macro: IRQ handler
%macro IRQ 2
global irq%1
irq%1:
    cli
    push dword 0        ; dummy error code
    push dword %2       ; interrupt number (32+N)
    jmp irq_common
%endmacro

;-----------------------------------------------------------
; CPU Exceptions 0-31
;-----------------------------------------------------------
ISR_NOERRCODE 0   ; Division by zero
ISR_NOERRCODE 1   ; Debug
ISR_NOERRCODE 2   ; NMI
ISR_NOERRCODE 3   ; Breakpoint
ISR_NOERRCODE 4   ; Overflow
ISR_NOERRCODE 5   ; Bound range exceeded
ISR_NOERRCODE 6   ; Invalid opcode
ISR_NOERRCODE 7   ; Device not available
ISR_ERRCODE   8   ; Double fault
ISR_NOERRCODE 9   ; Coprocessor segment overrun
ISR_ERRCODE   10  ; Invalid TSS
ISR_ERRCODE   11  ; Segment not present
ISR_ERRCODE   12  ; Stack fault
ISR_ERRCODE   13  ; General protection fault
ISR_ERRCODE   14  ; Page fault
ISR_NOERRCODE 15  ; Reserved
ISR_NOERRCODE 16  ; x87 FPU error
ISR_ERRCODE   17  ; Alignment check
ISR_NOERRCODE 18  ; Machine check
ISR_NOERRCODE 19  ; SIMD FP exception
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

;-----------------------------------------------------------
; Hardware IRQ 0-15 → interrupt 32-47
;-----------------------------------------------------------
IRQ  0, 32   ; PIT timer
IRQ  1, 33   ; Keyboard
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40   ; RTC
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44   ; PS/2 mouse
IRQ 13, 45
IRQ 14, 46   ; ATA primary
IRQ 15, 47   ; ATA secondary

;-----------------------------------------------------------
; Syscall — int 0x80
;-----------------------------------------------------------
global isr128
isr128:
    push dword 0
    push dword 128
    jmp isr_common

;-----------------------------------------------------------
; Common ISR stub
;-----------------------------------------------------------
isr_common:
    pusha
    push ds
    push es
    push fs
    push gs

    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pass registers_t* ke C handler
    call isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8          ; skip int_no dan err_code
    iret

;-----------------------------------------------------------
; Common IRQ stub
;-----------------------------------------------------------
irq_common:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret