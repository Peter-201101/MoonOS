; ==========================================
; MoonOS - Pure ASM Kernel
; Minimal bootable kernel with CLI shell
; Compatible with VirtualBox + QEMU
; ==========================================

[bits 64]

; ==========================================
; EXTERN & GLOBAL
; ==========================================
global asm_kernel_main
extern gdt_flush
extern idt_flush

; ==========================================
; CONSTANTS
; ==========================================

; I/O Ports
COM1_DATA       equ 0x3F8     ; Serial port data
COM1_STATUS     equ 0x3FD     ; Serial port status
COM1_CTRL       equ 0x3FC     ; Serial control register

; Keyboard
KEYBOARD_DATA   equ 0x60      ; PS/2 keyboard data port
KEYBOARD_STATUS equ 0x64      ; PS/2 keyboard status port

; PIC (8259 Programmable Interrupt Controller)
PIC1_CMD        equ 0x20      ; Master PIC command
PIC1_DATA       equ 0x21      ; Master PIC data
PIC2_CMD        equ 0xA0      ; Slave PIC command
PIC2_DATA       equ 0xA1      ; Slave PIC data

; IDT descriptor
IDT_SIZE        equ 256       ; 256 interrupt handlers

; ==========================================
; DATA SECTION
; ==========================================

section .data

; ---- GDT (Global Descriptor Table) ----
; Minimal GDT: null, code, data
gdt:
    dq 0                        ; Null selector (0x00)
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)  ; Code (0x08): present, s, type=11, L=1
    dq (1<<41) | (1<<44) | (1<<47) | (1<<54)  ; Data (0x10): present, s, type=2, D/B=1
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1        ; GDT size - 1
    dq gdt

; ---- IDT (Interrupt Descriptor Table) ----
idt_table:
    times IDT_SIZE dq 0         ; 256 interrupt entries (16 bytes each in 64-bit)

idt_descriptor:
    dw IDT_SIZE * 16 - 1        ; IDT size - 1
    dq idt_table

; ---- Keyboard buffer ----
kb_buffer:
    times 256 db 0              ; 256-byte keyboard buffer
kb_buffer_pos:
    dq 0                        ; Current position in buffer
kb_buffer_count:
    dq 0                        ; Number of chars in buffer

; ---- Shell command buffer ----
cmd_buffer:
    times 256 db 0              ; Command input buffer
cmd_buffer_pos:
    dq 0                        ; Current position

; ---- Welcome message ----
welcome_msg:
    db "MoonOS Pure ASM Kernel v1.0", 0x0A, 0x0D
    db "Type 'help' for commands", 0x0A, 0x0D, 0

prompt_msg:
    db "> ", 0

help_msg:
    db "Available commands: help, echo, clear, reboot, poweroff", 0x0A, 0x0D, 0

unknown_cmd_msg:
    db "Unknown command", 0x0A, 0x0D, 0

newline_msg:
    db 0x0A, 0x0D, 0

; ==========================================
; CODE SECTION
; ==========================================

section .text

; ==========================================
; ENTRY POINT (called from boot.asm)
; ==========================================
asm_kernel_main:
    ; Setup serial FIRST, before anything else
    call serial_init_simple
    
    ; Try to output something
    mov al, 'H'
    call serial_write_char_simple
    mov al, 'I'
    call serial_write_char_simple
    mov al, '\n'
    call serial_write_char_simple
    
    ; Halt
    cli
    hlt

; ==========================================
; SIMPLE SERIAL DRIVER (No GDT/IDT needed)
; ==========================================

; Initialize Serial Port (COM1) - Ultra-simple version
serial_init_simple:
    push rax
    push rdx
    
    ; Disable all interrupts
    mov al, 0x00
    mov dx, 0x3F9               ; COM1_CTRL + 1 (IER)
    out dx, al
    
    ; Set baud rate divisor to 1 (115200 baud)
    mov al, 0x80                ; Set DLAB
    mov dx, 0x3FB               ; COM1_CTRL + 3 (LCR)
    out dx, al
    
    mov al, 0x01                ; Divisor low byte
    mov dx, 0x3F8               ; COM1_DATA
    out dx, al
    
    mov al, 0x00                ; Divisor high byte
    mov dx, 0x3F9               ; COM1_DATA + 1
    out dx, al
    
    mov al, 0x03                ; 8N1
    mov dx, 0x3FB               ; COM1_CTRL + 3 (LCR)
    out dx, al
    
    pop rdx
    pop rax
    ret

; Serial: Write character - no blocking, just output
serial_write_char_simple:
    push rdx
    mov dx, 0x3FD               ; COM1_STATUS
    
    ; Wait for transmitter empty
    xor rcx, rcx
.wait_loop:
    in al, dx
    test al, 0x20               ; Transmitter Empty bit
    jnz .ready
    inc rcx
    cmp rcx, 100000             ; Timeout
    jl .wait_loop
    jmp .timeout
    
.ready:
    mov al, [rsp + 8]           ; Get char
    mov dx, 0x3F8               ; COM1_DATA
    out dx, al
    
.timeout:
    pop rdx
    ret

; ==========================================
; KEYBOARD DRIVER (STUB - NOT USED YET)
; ==========================================
; Removed for minimal build

; ==========================================
; PIC (STUB - NOT USED YET)
; ==========================================
; Removed for minimal build

; ==========================================
; IDT SETUP (STUB - NOT USED YET)
; ==========================================
; Removed for minimal build

; ==========================================
; SHELL (STUB - NOT USED YET)
; ==========================================
; Removed for minimal build
