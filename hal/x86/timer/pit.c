#include <hal/x86/timer/pit.h>
#include <hal/x86/interrupt/irq.h>
#include <core/scheduler.h>
#include <hal/x86/serial/serial_debug.h>

#define PIT_CHANNEL0    0x40
#define PIT_CMD         0x43
#define PIT_FREQUENCY   1193182  /* Hz */
#define PIT_HZ          100      /* target: 100 ticks/detik */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static volatile uint32_t pit_ticks = 0;

static void pit_irq_handler(registers_t *regs)
{
    (void)regs;
    pit_ticks++;
    scheduler_tick();
}

void pit_init(void)
{
    uint32_t divisor = PIT_FREQUENCY / PIT_HZ;

    /* Mode 3: square wave, channel 0 */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    /* Register IRQ0 = timer */
    irq_register(0, pit_irq_handler);

    serial_printf("[PIT] Timer at %u Hz\n", PIT_HZ);
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void pit_sleep(uint32_t ms)
{
    uint32_t target = pit_ticks + (ms * PIT_HZ / 1000);
    while (pit_ticks < target)
        __asm__ volatile("hlt");
}