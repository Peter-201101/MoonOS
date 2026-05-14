#include "timer.h"
#include <hal/arm/interrupt/irq.h>
#include <hal/arm/serial/uart.h>
#include <core/scheduler.h>

#define ARM_TIMER_HZ 100

static volatile uint64_t ticks = 0;

static void timer_irq_handler(uint32_t irq)
{
    (void)irq;
    ticks++;
    scheduler_tick();

    /* Reset timer */
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    uint64_t val = freq / ARM_TIMER_HZ;
    __asm__ volatile("msr cntp_tval_el0, %0" : : "r"(val));
}

void arm_timer_init(void)
{
    /* Ambil frekuensi timer */
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));

    /* Set interval */
    uint64_t val = freq / ARM_TIMER_HZ;
    __asm__ volatile("msr cntp_tval_el0, %0" : : "r"(val));

    /* Enable timer */
    __asm__ volatile("msr cntp_ctl_el0, %0" : : "r"((uint64_t)1));

    /* Register IRQ — ARM timer = IRQ 30 */
    arm_irq_register(30, timer_irq_handler);

    uart_printf("[TIMER] ARM timer at %u Hz\n", ARM_TIMER_HZ);
}

uint64_t arm_timer_get_ticks(void)
{
    return ticks;
}