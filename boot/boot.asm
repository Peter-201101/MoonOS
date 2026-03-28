[bits 32]

section .multiboot2
align 8
multiboot_header:
    dd 0xe85250d6                ; magic number
    dd 0                         ; architecture 0 (i386)
    dd header_end - multiboot_header
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - multiboot_header))
    dw 0
    dw 0
    dd 8
header_end:

section .bss
align 4096
p4_table: resb 4096
p3_table: resb 4096
p2_table: resb 4096

align 16
stack_bottom: resb 16384
stack_top:

section .text
global _start
extern asm_kernel_main

_start:
    mov esp, stack_top
    mov edi, eax                ; Simpan magic multiboot
    mov esi, ebx                ; Simpan pointer multiboot

    ; 1. Setup Paging sederhana (Identity Map 1GB pertama)
    call setup_page_tables
    call enable_paging

    ; 2. Load 64-bit GDT
    lgdt [gdt64.pointer]

    ; 3. Jump ke Long Mode!
    jmp gdt64.code:long_mode_start

setup_page_tables:
    ; Map P4 ke P3
    mov eax, p3_table
    or eax, 0b11                ; present + writable
    mov [p4_table], eax

    ; Map P3 ke P2
    mov eax, p2_table
    or eax, 0b11                ; present + writable
    mov [p3_table], eax

    ; Map P2 ke 1GB pertama menggunakan 2MB Huge Pages
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000           ; 2MB
    mul ecx
    or eax, 0b10000011          ; present + writable + huge
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne .map_p2_table
    ret

enable_paging:
    ; Load P4 ke CR3
    mov eax, p4_table
    mov cr3, eax

    ; Enable PAE di CR4
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Set long mode bit di EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging di CR0
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    ret

[bits 64]
long_mode_start:
    ; Load data segment ke semua segment registers
    mov ax, gdt64.data
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Setup stack pointer (16-byte aligned)
    mov rsp, stack_top
    and rsp, -16

    ; Panggil kernel
    call asm_kernel_main

    cli
.hlt:
    hlt
    jmp .hlt

; ==========================================
; GDT 64-bit (FIXED)
; CRITICAL: Untuk code segment 64-bit:
;   - L bit (bit 53) = 1  → 64-bit mode
;   - D/B bit (bit 54) = 0 → WAJIB 0 jika L=1
;   VirtualBox/hardware reject jika keduanya 1
; ==========================================
section .rodata
gdt64:
    dq 0                                        ; Null segment
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)   ; Code: C=1, S=1, P=1, L=1, D=0
.data: equ $ - gdt64
    dq (1<<41) | (1<<44) | (1<<47) | (1<<54)   ; Data: W=1, S=1, P=1, D/B=1
.pointer:
    dw $ - gdt64 - 1
    dq gdt64