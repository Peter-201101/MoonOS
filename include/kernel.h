#ifndef KERNEL_H
#define KERNEL_H

#include <types.h>
#include <stdarg.h>

#define KERNEL_NAME     "MoonOS"
#define KERNEL_VERSION  "0.1.0"

#define LOG_INFO  "[INFO] "
#define LOG_OK    "[OK]   "
#define LOG_WARN  "[WARN] "
#define LOG_ERR   "[ERR]  "

#ifdef ARCH_X86
#include <hal/x86/serial/serial_debug.h>
#include <display/display.h>   // untuk vdisplay_printf

// klog sekarang memanggil fungsi yang mengirim ke display + serial
void klog_printf(const char *fmt, ...);

#define klog(...) klog_printf(__VA_ARGS__)

#endif

#ifdef ARCH_ARM
#include <hal/arm/serial/uart.h>
#define kprint  uart_puts
#define kprintf uart_printf
#define klog(fmt, ...) uart_printf(fmt, ##__VA_ARGS__)
#endif

#endif