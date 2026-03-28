#ifndef SHUTDOWN_HPP
#define SHUTDOWN_HPP

#include <types.hpp> // Pastikan kamu punya file types untuk uint16_t dsb.

/**
 * @brief Menyiapkan sistem sebelum dimatikan.
 * Digunakan untuk sinkronisasi file sistem atau menutup resource hardware.
 */
void prepare_shutdown();

/**
 * @brief Melakukan proses mematikan komputer (Power Off).
 * Mencoba berbagai metode ACPI untuk emulator (QEMU/VirtualBox) 
 * dan melakukan CPU Halt sebagai cadangan.
 */
void shutdown();

#endif