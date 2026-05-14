#include "panic.h"
#include <hal/x86/serial/serial_debug.h>

static inline void arch_halt(void)
{
#ifdef ARCH_X86
    __asm__ volatile("cli; hlt");
#endif
#ifdef ARCH_ARM
    __asm__ volatile("msr daifset, #2"); /* disable interrupts */
    __asm__ volatile("wfe");             /* wait for event = low power halt */
#endif
}

void panic(const char *msg, const char *file, uint32_t line)
{
    __asm__ volatile(
#ifdef ARCH_X86
        "cli"
#endif
#ifdef ARCH_ARM
        "msr daifset, #2"
#endif
    );

#ifdef ARCH_X86
    serial_puts("\n\n========== KERNEL PANIC ==========\n");
    serial_puts("Message : "); serial_puts(msg);  serial_puts("\n");
    serial_puts("File    : "); serial_puts(file); serial_puts("\n");
    serial_putint(line);
    serial_puts("\n===================================\n");
#endif

#ifdef ARCH_ARM
    /* ARM pakai UART */
    extern void uart_puts(const char *s);
    extern void uart_putchar(char c);
    uart_puts("\n\n========== KERNEL PANIC ==========\n");
    uart_puts("Message : "); uart_puts(msg);  uart_puts("\n");
    uart_puts("File    : "); uart_puts(file); uart_puts("\n");
    uart_puts("===================================\n");
#endif

    (void)line;

    for (;;) arch_halt();
}