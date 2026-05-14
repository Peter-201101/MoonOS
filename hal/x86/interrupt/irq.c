#include <hal/x86/interrupt/irq.h>
#include <hal/x86/serial/serial_debug.h>

/* Exception names untuk debug */
static const char *exception_names[] = {
    "Division by Zero",       "Debug",
    "NMI",                    "Breakpoint",
    "Overflow",               "Bound Range Exceeded",
    "Invalid Opcode",         "Device Not Available",
    "Double Fault",           "Coprocessor Segment Overrun",
    "Invalid TSS",            "Segment Not Present",
    "Stack Fault",            "General Protection Fault",
    "Page Fault",             "Reserved",
    "x87 FPU Error",          "Alignment Check",
    "Machine Check",          "SIMD FP Exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
};

/* Registered IRQ handlers */
static irq_handler_fn_t irq_handlers[IRQ_COUNT] = {0};

/* Forward declare */
void pic_send_eoi(uint8_t irq);
void pic_init(void);
void pic_unmask_irq(uint8_t irq);

void irq_init(void)
{
    pic_init();

    /* Mask semua IRQ dulu */
    for (uint8_t i = 0; i < IRQ_COUNT; i++)
        irq_handlers[i] = NULL;

    serial_puts("[IRQ] Initialized\n");
}

void irq_register(uint8_t irq, irq_handler_fn_t handler)
{
    if (irq >= IRQ_COUNT) return;
    irq_handlers[irq] = handler;
    pic_unmask_irq(irq);
    serial_printf("[IRQ] Registered handler for IRQ%u\n", irq);
}

void irq_unregister(uint8_t irq)
{
    if (irq >= IRQ_COUNT) return;
    irq_handlers[irq] = NULL;
}

/* Dipanggil dari isr.asm untuk exceptions */
void isr_handler(registers_t *regs)
{
    if (regs->int_no == 128) {
        /* Syscall */
        extern void syscall_dispatch(void *regs);
        syscall_dispatch(regs);
        return;
    }

    serial_printf("\n[EXCEPTION] %s (int %u)\n",
        regs->int_no < 32 ? exception_names[regs->int_no] : "Unknown",
        regs->int_no);
    serial_printf("  EIP: %x  CS: %x  EFLAGS: %x\n",
        regs->eip, regs->cs, regs->eflags);
    serial_printf("  EAX: %x  EBX: %x  ECX: %x  EDX: %x\n",
        regs->eax, regs->ebx, regs->ecx, regs->edx);
    serial_printf("  ESP: %x  EBP: %x  ESI: %x  EDI: %x\n",
        regs->esp, regs->ebp, regs->esi, regs->edi);
    serial_printf("  ERR: %x\n", regs->err_code);

    for (;;) __asm__ volatile("cli; hlt");
}

/* Dipanggil dari isr.asm untuk hardware IRQ */
void irq_handler(registers_t *regs)
{
    uint8_t irq = regs->int_no - 32;

    if (irq_handlers[irq])
        irq_handlers[irq](regs);

    pic_send_eoi(irq);
}