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
stack_bottom: resb 16384
stack_top:

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top
    mov edi, eax                ; Simpan magic multiboot
    mov esi, ebx                ; Simpan pointer multiboot

    ; 1. Setup Paging sederhana (Identity Map 2MB pertama)
    call setup_page_tables
    call enable_paging

    ; 2. Load 64-bit GDT
    lgdt [gdt64.pointer]

    ; 3. Jump ke Long Mode!
    jmp gdt64.code:long_mode_start

setup_page_tables:
    ; Map P4 ke P3
    mov eax, p3_table
    or eax, 0b11 ; present + writable
    mov [p4_table], eax

    ; Map P3 ke P2
    mov eax, p2_table
    or eax, 0b11 ; present + writable
    mov [p3_table], eax

    ; Map P2 ke 1GB pertama menggunakan 2MB Huge Pages
    mov ecx, 0         ; counter loop
.map_p2_table:
    mov eax, 0x200000  ; 2MB
    mul ecx            ; eax = 2MB * ecx
    or eax, 0b10000011 ; present + writable + huge
    mov [p2_table + ecx * 8], eax

    inc ecx            ; counter++
    cmp ecx, 512       ; apakah sudah 512 entri? (512 * 2MB = 1GB)
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
    ; Load null ke data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Panggil kernel utama dengan argumen multiboot
    mov edi, edi                ; EDI masih pegang magic (32-bit ke 64-bit)
    mov rsi, rsi                ; RSI masih pegang pointer
    call kernel_main

    cli
.hlt:
    hlt
    jmp .hlt

section .rodata
gdt64:
    dq 0 ; zero entry
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64