// ==========================================
// MoonOS - keyboard.cpp
// PS/2 Keyboard driver
// NOTE: Stub dulu, isi setelah IDT ready
// ==========================================

#include "keyboard.hpp"
#include "serial.hpp"
#include <utils/io.hpp>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

namespace Keyboard {

    // Scancode set 1 → ASCII mapping (simplified)
    static const char scancode_map[128] = {
        0, 0, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0,'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,'\\','z','x','c','v','b','n','m',',','.','/',0,
        '*',0,' '
    };

    void init() {
        // Flush keyboard buffer
        while (inb(0x64) & 0x01) inb(0x60);
    }

    bool has_input() {
        return (inb(0x64) & 0x01) != 0;
    }

    char get_char() {
        while (!has_input());
        uint8_t scancode = inb(0x60);
        if (scancode & 0x80) return 0; // key release, skip
        if (scancode < 128) return scancode_map[scancode];
        return 0;
    }

    extern "C" void keyboard_handler_main() {
    // 1. Baca data dari port keyboard
    uint8_t scancode = IO::inb(0x60);
    
    // 2. Debug Log: Cetak kode hex yang diterima
    Serial::write("[DEBUG] Key Received: ");
    Serial::write_hex(scancode);
    Serial::write_char('\n');

    // 3. Logika mapping (hanya untuk tombol yang ditekan)
    if (!(scancode & 0x80)) { 
        char c = Keyboard::scancode_map[scancode];
        if (c) {
            Serial::write_char(c);
        }
    }
}

} // namespace Keyboard
