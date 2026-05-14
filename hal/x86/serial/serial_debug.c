#include "serial_debug.h"

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

static inline bool serial_tx_empty(void)
{
    return (inb(SERIAL_DEBUG_PORT + 5) & 0x20) != 0;
}

void serial_debug_init(void)
{
    outb(SERIAL_DEBUG_PORT + 1, 0x00);
    outb(SERIAL_DEBUG_PORT + 3, 0x80);
    outb(SERIAL_DEBUG_PORT + 0, 0x03);
    outb(SERIAL_DEBUG_PORT + 1, 0x00);
    outb(SERIAL_DEBUG_PORT + 3, 0x03);
    outb(SERIAL_DEBUG_PORT + 2, 0xC7);
    outb(SERIAL_DEBUG_PORT + 4, 0x0B);
}

void serial_putchar(char c)
{
    while (!serial_tx_empty());
    outb(SERIAL_DEBUG_PORT, (uint8_t)c);
}

void serial_puts(const char *s)
{
    while (*s) serial_putchar(*s++);
}

void serial_putint(uint32_t n)
{
    char buf[12]; int i = 0;
    if (!n) { serial_putchar('0'); return; }
    while (n) { buf[i++] = '0' + (n % 10); n /= 10; }
    while (i--) serial_putchar(buf[i]);
}

void serial_puthex(uint32_t n)
{
    static const char h[] = "0123456789ABCDEF";
    serial_puts("0x");
    for (int i = 28; i >= 0; i -= 4)
        serial_putchar(h[(n >> i) & 0xF]);
}

void serial_vprintf(const char *fmt, va_list args)
{
    while (*fmt) {
        if (*fmt != '%') { serial_putchar(*fmt++); continue; }
        fmt++;
        switch (*fmt++) {
            case 'd': {
                int32_t v = va_arg(args, int32_t);
                if (v < 0) { serial_putchar('-'); v = -v; }
                serial_putint((uint32_t)v);
                break;
            }
            case 'u': serial_putint(va_arg(args, uint32_t)); break;
            case 'x': serial_puthex(va_arg(args, uint32_t)); break;
            case 's': serial_puts(va_arg(args, const char *)); break;
            case 'c': serial_putchar((char)va_arg(args, int)); break;
            case '%': serial_putchar('%'); break;
            default:  serial_putchar('?'); break;
        }
    }
}

void serial_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    serial_vprintf(fmt, args);
    va_end(args);
}