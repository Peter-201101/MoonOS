; MoonOS - isr.asm
[bits 64]

global gdt_flush
global idt_flush
global irq1_handler
global default_isr_handler    ; <--- TAMBAHKAN INI

extern keyboard_handler_main

section .text

; --- Flush GDT ---
gdt_flush:
    lgdt [rdi]              
    mov ax, 0x10            
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08               
    lea rax, [rel .done]
    push rax
    retfq
.done:
    ret

; --- Flush IDT ---
idt_flush:
    lidt [rdi]
    ret

; --- Default Handler (Pengaman) ---
default_isr_handler:
    push rax
    ; Kirim EOI ke PIC (End of Interrupt) agar PIC tidak nge-hang
    mov al, 0x20
    out 0x20, al
    out 0xA0, al
    pop rax
    iretq

; --- Handler Keyboard (IRQ 1) ---
irq1_handler:
    ; 1. Simpan register lengkap (64-bit)
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    ; 2. Panggil C++
    call keyboard_handler_main

    ; 3. Kirim EOI ke PIC Master (Port 0x20)
    mov al, 0x20
    out 0x20, al

    ; 4. Kembalikan register
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rax
    
    iretq