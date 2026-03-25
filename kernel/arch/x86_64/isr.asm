; MoonOS - isr.asm
[bits 64]

global gdt_flush
global idt_flush
global irq1_handler           ; Tambahkan ini agar bisa dipanggil di idt.cpp
extern keyboard_handler_main  ; Fungsi C++ yang akan kita panggil

; Flush GDT dan reload segment registers
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

; Flush IDT
idt_flush:
    lidt [rdi]
    ret

; --- Handler untuk Keyboard (IRQ 1) ---
irq1_handler:
    ; 1. Simpan register agar tidak rusak saat balik ke kernel_main
    push rax
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    ; 2. Panggil logika C++
    call keyboard_handler_main

    ; 3. Beritahu PIC kalau interupsi sudah selesai (EOI - End of Interrupt)
    ; Jika tidak dikirim, keyboard hanya akan berfungsi sekali saja.
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
    
    ; 5. Return khusus interupsi (Interrupt Return 64-bit)
    iretq