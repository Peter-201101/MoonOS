#ifndef GIC_H
#define GIC_H

#include <types.h>

/* GIC base untuk Raspberry Pi 3 */
#define GIC_BASE        0x40000000UL
#define GICD_BASE       (GIC_BASE + 0x1000)
#define GICC_BASE       (GIC_BASE + 0x2000)

void gic_init(void);
void gic_enable_irq(uint32_t irq);
void gic_disable_irq(uint32_t irq);
void gic_send_eoi(uint32_t irq);

#endif