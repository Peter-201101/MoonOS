#ifndef KERNEL_H
#define KERNEL_H

#include <types.h>

#define KERNEL_NAME     "MoonOS"
#define KERNEL_VERSION  "0.1.0"

#define LOG_INFO  "[INFO] "
#define LOG_OK    "[OK]   "
#define LOG_WARN  "[WARN] "
#define LOG_ERR   "[ERR]  "

/* Abstract early print — diimplementasi per arch */
#ifdef ARCH_X86
#include <hal/x86/serial/serial_debug.h>
#define kprint  serial_puts
#define kprintf serial_printf
#define klog(fmt, ...) serial_printf(fmt, ##__VA_ARGS__)
#endif

#ifdef ARCH_ARM
#include <hal/arm/serial/uart.h>
#define kprint  uart_puts
#define kprintf uart_printf
#define klog(fmt, ...) uart_printf(fmt, ##__VA_ARGS__)
#endif

#endif