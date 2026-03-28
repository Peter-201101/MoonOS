#pragma once

// ==========================================
// MoonOS - serial.hpp
// Output debug via serial port (COM1)
// Di QEMU tampil langsung di terminal
// ==========================================

#include "../include/types.hpp"

namespace Serial {

    // COM1 port address
    static const uint16_t COM1 = 0x3F8;

    void init();
    void write_char(char c);
    void write(const char* str);
    void writeln(const char* str);
    void write_hex(uint64_t value);
    void write_dec(uint64_t value);

} // namespace Serial
