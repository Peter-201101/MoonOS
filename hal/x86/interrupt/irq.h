#ifndef IRQ_H
#define IRQ_H

#include <types.h>
#include <hal/x86/interrupt/idt.h>

#define IRQ_COUNT 16

typedef void (*irq_handler_fn_t)(registers_t *regs);

void irq_init(void);
void irq_register(uint8_t irq, irq_handler_fn_t handler);
void irq_unregister(uint8_t irq);

/* Dipanggil dari isr.asm */
void irq_handler(registers_t *regs);

/* Dipanggil dari isr.asm untuk exceptions */
void isr_handler(registers_t *regs);

#endif