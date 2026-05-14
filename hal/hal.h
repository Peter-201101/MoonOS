#ifndef HAL_H
#define HAL_H

#include "../include/config.h"

// Fungsi standar yang harus diimplementasikan setiap arsitektur
void hal_init();
void hal_cpu_halt();
void hal_interrupt_enable();
void hal_interrupt_disable();

#endif