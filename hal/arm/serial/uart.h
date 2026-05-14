#ifndef UART_H
#define UART_H

#include <types.h>

/* Raspberry Pi 3 PL011 UART */
#define UART0_BASE  0x3F201000UL

void uart_init(void);
void uart_putchar(char c);
void uart_puts(const char *s);
void uart_printf(const char *fmt, ...);

#endif