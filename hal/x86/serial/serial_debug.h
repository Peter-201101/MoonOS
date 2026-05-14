#ifndef SERIAL_DEBUG_H
#define SERIAL_DEBUG_H

#include <types.h>
#include <stdarg.h>

#define SERIAL_DEBUG_PORT 0x3F8

void serial_debug_init(void);
void serial_putchar(char c);
void serial_puts(const char *str);
void serial_putint(uint32_t n);
void serial_puthex(uint32_t n);
void serial_printf(const char *fmt, ...);
void serial_vprintf(const char *fmt, va_list args);

#endif