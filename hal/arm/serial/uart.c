#include "uart.h"
#include <stdarg.h>
#define UART_DR   (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR   (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART_IBRD (*(volatile uint32_t *)(UART0_BASE + 0x24))
#define UART_FBRD (*(volatile uint32_t *)(UART0_BASE + 0x28))
#define UART_LCRH (*(volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART_CR   (*(volatile uint32_t *)(UART0_BASE + 0x30))
#define UART_IMSC (*(volatile uint32_t *)(UART0_BASE + 0x38))
#define UART_ICR  (*(volatile uint32_t *)(UART0_BASE + 0x44))

/* GPIO registers */
#define GPIO_BASE       0x3F200000UL
#define GPFSEL1         (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPPUD           (*(volatile uint32_t *)(GPIO_BASE + 0x94))
#define GPPUDCLK0       (*(volatile uint32_t *)(GPIO_BASE + 0x98))

static void delay(uint32_t count)
{
    while (count--) __asm__ volatile("nop");
}

void uart_init(void)
{
    /* Disable UART */
    UART_CR = 0;

    /* Setup GPIO 14 dan 15 untuk UART */
    uint32_t sel = GPFSEL1;
    sel &= ~((7 << 12) | (7 << 15));
    sel |=  ((4 << 12) | (4 << 15)); /* Alt0 = UART */
    GPFSEL1 = sel;

    GPPUD = 0;
    delay(150);
    GPPUDCLK0 = (1 << 14) | (1 << 15);
    delay(150);
    GPPUDCLK0 = 0;

    /* Clear interrupts */
    UART_ICR = 0x7FF;

    /* Baud rate 115200 @ 3MHz clock */
    UART_IBRD = 1;
    UART_FBRD = 40;

    /* 8 bit, no parity, 1 stop, FIFO enable */
    UART_LCRH = (1 << 4) | (1 << 5) | (1 << 6);

    /* Disable interrupts */
    UART_IMSC = 0;

    /* Enable UART, TX, RX */
    UART_CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putchar(char c)
{
    while (UART_FR & (1 << 5));
    UART_DR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') uart_putchar('\r');
        uart_putchar(*s++);
    }
}

static void uart_putuint(uint32_t n, uint32_t base)
{
    static const char d[] = "0123456789ABCDEF";
    char buf[32]; int i = 0;
    if (!n) { uart_putchar('0'); return; }
    while (n) { buf[i++] = d[n % base]; n /= base; }
    while (i--) uart_putchar(buf[i]);
}

void uart_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt != '%') { uart_putchar(*fmt++); continue; }
        fmt++;
        switch (*fmt++) {
            case 'd': { int32_t v = va_arg(args, int32_t); if(v<0){uart_putchar('-');v=-v;} uart_putuint((uint32_t)v,10); break; }
            case 'u': uart_putuint(va_arg(args, uint32_t), 10); break;
            case 'x': uart_puts("0x"); uart_putuint(va_arg(args, uint32_t), 16); break;
            case 's': uart_puts(va_arg(args, const char *)); break;
            case 'c': uart_putchar((char)va_arg(args, int)); break;
            case '%': uart_putchar('%'); break;
            default:  uart_putchar('?'); break;
        }
    }
    va_end(args);
}

/* ====== Wrapper agar kompatibel dengan kode lain ====== */
void serial_putchar(char c) {
    uart_putchar(c);
}

void serial_puts(const char *s) {
    uart_puts(s);
}

void serial_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // gunakan buffer + vsnprintf agar aman
    uart_printf;
    va_end(args);
}
