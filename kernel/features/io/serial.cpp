// ==========================================
// MoonOS - serial.cpp
// Output debug via serial port COM1
// ==========================================

#include "serial.hpp"

// Helper: tulis ke I/O port
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Helper: baca dari I/O port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

namespace Serial {

    void init() {
        outb(COM1 + 1, 0x00); // Disable interrupts
        outb(COM1 + 3, 0x80); // Enable DLAB (baud rate mode)
        outb(COM1 + 0, 0x03); // Baud rate low byte: 38400
        outb(COM1 + 1, 0x00); // Baud rate high byte
        outb(COM1 + 3, 0x03); // 8 bits, no parity, 1 stop bit
        outb(COM1 + 2, 0xC7); // Enable FIFO
        outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    }

    static bool is_transmit_empty() {
        return inb(COM1 + 5) & 0x20;
    }

    void write_char(char c) {
        while (!is_transmit_empty());
        outb(COM1, (uint8_t)c);
    }

    void write(const char* str) {
        for (int i = 0; str[i] != '\0'; i++) {
            write_char(str[i]);
        }
    }

    void writeln(const char* str) {
        write(str);
        write_char('\n');
    }

    void write_hex(uint64_t value) {
        write("0x");
        const char* hex = "0123456789ABCDEF";
        bool leading = true;
        for (int i = 60; i >= 0; i -= 4) {
            uint8_t nibble = (value >> i) & 0xF;
            if (nibble != 0) leading = false;
            if (!leading) write_char(hex[nibble]);
        }
        if (leading) write_char('0');
    }

    void write_dec(uint64_t value) {
        if (value == 0) { write_char('0'); return; }
        char buf[20];
        int i = 0;
        while (value > 0) {
            buf[i++] = '0' + (value % 10);
            value /= 10;
        }
        for (int j = i - 1; j >= 0; j--) write_char(buf[j]);
    }

} // namespace Serial
