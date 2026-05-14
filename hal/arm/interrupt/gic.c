#include "gic.h"
#include <hal/arm/serial/uart.h>

#define GICD_CTLR       (*(volatile uint32_t *)(GICD_BASE + 0x000))
#define GICD_ISENABLER  ((volatile uint32_t *)(GICD_BASE + 0x100))
#define GICD_ICENABLER  ((volatile uint32_t *)(GICD_BASE + 0x180))
#define GICC_CTLR       (*(volatile uint32_t *)(GICC_BASE + 0x000))
#define GICC_PMR        (*(volatile uint32_t *)(GICC_BASE + 0x004))
#define GICC_EOIR       (*(volatile uint32_t *)(GICC_BASE + 0x010))

void gic_init(void)
{
    /* Enable distributor */
    GICD_CTLR = 1;

    /* Set priority mask — terima semua */
    GICC_PMR = 0xFF;

    /* Enable CPU interface */
    GICC_CTLR = 1;

    uart_puts("[GIC] Initialized\n");
}

void gic_enable_irq(uint32_t irq)
{
    GICD_ISENABLER[irq / 32] = (1u << (irq % 32));
}

void gic_disable_irq(uint32_t irq)
{
    GICD_ICENABLER[irq / 32] = (1u << (irq % 32));
}

void gic_send_eoi(uint32_t irq)
{
    GICC_EOIR = irq;
}