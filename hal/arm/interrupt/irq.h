#ifndef ARM_IRQ_H
#define ARM_IRQ_H

#include <types.h>

typedef void (*arm_irq_handler_t)(uint32_t irq);

void arm_irq_init(void);
void arm_irq_register(uint32_t irq, arm_irq_handler_t handler);
void arm_irq_handler(void);

#endif