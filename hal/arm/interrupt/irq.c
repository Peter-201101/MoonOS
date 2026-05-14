#include "irq.h"
#include "gic.h"
#include <hal/arm/serial/uart.h>

#define MAX_ARM_IRQS 256

static arm_irq_handler_t irq_handlers[MAX_ARM_IRQS] = {0};

#define GICC_IAR (*(volatile uint32_t *)(GICC_BASE + 0x00C))
#define GICC_EOIR (*(volatile uint32_t *)(GICC_BASE + 0x010))

void arm_irq_init(void)
{
    gic_init();
    uart_puts("[IRQ] ARM IRQ initialized\n");
}

void arm_irq_register(uint32_t irq, arm_irq_handler_t handler)
{
    if (irq >= MAX_ARM_IRQS) return;
    irq_handlers[irq] = handler;
    gic_enable_irq(irq);
}

void arm_irq_handler(void)
{
    uint32_t irq = GICC_IAR & 0x3FF;
    if (irq < MAX_ARM_IRQS && irq_handlers[irq])
        irq_handlers[irq](irq);
    GICC_EOIR = irq;
}