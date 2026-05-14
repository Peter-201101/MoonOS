#include <core/syscall.h>        /* ← harus pertama */
#include <core/syscall_table.h>  /* ← baru ini */
#include <core/panic.h>
#include <include/kernel.h>

/*
 * syscall_dispatch — dipanggil dari ISR int 0x80
 *
 * Flow:
 *   userspace: mov eax, SYS_XXX → int 0x80
 *   → IDT entry 0x80 → isr_syscall (assembly)
 *   → syscall_dispatch (sini)
 *   → handler di syscall_table
 *   → return value di regs->eax
 *   → isr_syscall restore registers
 *   → userspace lanjut
 */
void syscall_dispatch(syscall_regs_t *regs)
{
    uint32_t num = regs->eax;

    syscall_fn_t handler = syscall_table_get(num);

    if (!handler) {
        klog(LOG_WARN "syscall: unknown syscall %u\n", num);
        regs->eax = (uint32_t)SYS_EINVAL;
        return;
    }

    int32_t result = handler(regs);
    regs->eax = (uint32_t)result;
}

void syscall_init(void)
{
    syscall_table_init();

    /*
     * Register int 0x80 ke IDT.
     * Dipanggil setelah idt_init() — nanti waktu HAL phase.
     * Untuk sekarang, ini placeholder.
     */

    klog(LOG_OK "Syscall: int 0x80 handler ready\n");
}