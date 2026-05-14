; core/switch_context.asm
; void switch_context(cpu_context_t *old_ctx, cpu_context_t *new_ctx)
;
; x86 calling convention:
;   [esp+4] = old_ctx (bisa NULL kalau task pertama)
;   [esp+8] = new_ctx

BITS 32

section .note.GNU-stack noalloc noexec nowrite progbits

section .text
global switch_context

switch_context:
    mov eax, [esp+4]    ; old_ctx
    mov edx, [esp+8]    ; new_ctx

    ; Simpan context lama (kalau old_ctx != NULL)
    test eax, eax
    jz .restore

    ; Simpan general purpose registers ke old_ctx
    mov [eax+0],  edi
    mov [eax+4],  esi
    mov [eax+8],  ebp
    mov [eax+12], esp
    mov [eax+16], ebx
    mov [eax+20], edx   ; simpan edx dulu (overwrite nanti tidak masalah)
    mov [eax+24], ecx
    mov [eax+28], eax   ; simpan eax (nilainya = old_ctx, tidak penting)

    ; Simpan EIP — pakai trik: push return address, pop ke context
    mov ecx, [esp]      ; ambil return address dari stack
    mov [eax+32], ecx   ; simpan sebagai EIP

    ; Simpan EFLAGS
    pushfd
    pop ecx
    mov [eax+36], ecx

.restore:
    ; Restore context baru dari new_ctx
    mov edi, [edx+0]
    mov esi, [edx+4]
    mov ebp, [edx+8]
    mov esp, [edx+12]   ; switch stack!
    mov ebx, [edx+16]
    mov ecx, [edx+24]
    mov eax, [edx+28]

    ; Restore EFLAGS
    push dword [edx+36]
    popfd

    ; Jump ke EIP task baru
    jmp dword [edx+32]