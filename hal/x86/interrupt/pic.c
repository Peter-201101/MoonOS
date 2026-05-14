#include <hal/x86/interrupt/irq.h>
#include <hal/x86/serial/serial_debug.h>

/* PIC ports */
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

/* PIC commands */
#define PIC_EOI     0x20    /* End of interrupt */
#define PIC_INIT    0x11    /* Initialize + ICW4 needed */
#define PIC_8086    0x01    /* 8086 mode */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

void pic_init(void)
{
    /* Save masks */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* Init sequence */
    outb(PIC1_CMD,  PIC_INIT); io_wait();
    outb(PIC2_CMD,  PIC_INIT); io_wait();

    /* Vector offsets: IRQ0-7 → int 32, IRQ8-15 → int 40 */
    outb(PIC1_DATA, 0x20); io_wait();
    outb(PIC2_DATA, 0x28); io_wait();

    /* Cascade */
    outb(PIC1_DATA, 0x04); io_wait(); /* IRQ2 = slave */
    outb(PIC2_DATA, 0x02); io_wait(); /* slave ID = 2 */

    /* 8086 mode */
    outb(PIC1_DATA, PIC_8086); io_wait();
    outb(PIC2_DATA, PIC_8086); io_wait();

    /* Restore masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);

    serial_puts("[PIC] Initialized\n");
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_mask_irq(uint8_t irq)
{
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq % 8;
    outb(port, inb(port) | (1 << bit));
}

void pic_unmask_irq(uint8_t irq)
{
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t  bit  = irq % 8;
    outb(port, inb(port) & ~(1 << bit));
}