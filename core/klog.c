#include <kernel.h>
#include <display/display.h>
#include <stdarg.h>

void klog_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vdisplay_printf(fmt, args);
    va_end(args);
}