// ==========================================
// MoonOS - keyboard.cpp
// PS/2 Keyboard driver with extended support
// Arrow keys, Shift, Ctrl modifiers
// ==========================================

#include "keyboard.hpp"
#include "serial.hpp"
#include <utils/io.hpp>
#include <utils/circular_buffer.hpp>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Scancode set 1 → ASCII mapping (lowercase)
static const char keyboard_scancode_map[128] = {
    0, 0, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*',0,' '
};

// Scancode set 1 → ASCII mapping (uppercase - with shift)
static const char keyboard_scancode_map_shift[128] = {
    0, 0, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"','~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,
    '*',0,' '
};

namespace Keyboard {

    // Input buffer for CLI mode
    static Utils::CircularBuffer<char, 512> input_buffer;
    static bool shift_pressed = false;
    static bool ctrl_pressed = false;
    static bool alt_pressed = false;
    static bool extended_code = false; // For 0xE0 prefixed keys

    void init() {
        // Flush keyboard buffer
        while (inb(0x64) & 0x01) inb(0x60);
    }

    bool has_input() {
        return !input_buffer.is_empty();
    }

    bool try_read(char& c) {
        return input_buffer.pop(c);
    }

    char get_char() {
        // Blocking read
        while (!has_input()) {
            __asm__ volatile("hlt");
        }
        char c;
        input_buffer.pop(c);
        return c;
    }

    void put_buffer(char c) {
        input_buffer.push(c);
    }

    bool is_shift_held() {
        return shift_pressed;
    }

    bool is_ctrl_held() {
        return ctrl_pressed;
    }

}

// External C handler for assembly interrupt dispatcher
extern "C" void keyboard_handler_main() {
    using namespace Keyboard;
    
    uint8_t scancode = inb(0x60);
    
    // Handle extended scancodes (arrow keys, etc)
    if (scancode == 0xE0) {
        extended_code = true;
        return;
    }
    
    // Arrow keys and extended keys (0xE0 prefix)
    if (extended_code) {
        extended_code = false;
        
        if (!(scancode & 0x80)) { // Key press
            switch (scancode) {
                case 0x48: // Up arrow
                    Keyboard::put_buffer((char)KeyCode::KEY_UP);
                    Serial::write_char('^');
                    return;
                case 0x50: // Down arrow
                    Keyboard::put_buffer((char)KeyCode::KEY_DOWN);
                    Serial::write_char('v');
                    return;
                case 0x4B: // Left arrow
                    Keyboard::put_buffer((char)KeyCode::KEY_LEFT);
                    Serial::write_char('<');
                    return;
                case 0x4D: // Right arrow
                    Keyboard::put_buffer((char)KeyCode::KEY_RIGHT);
                    Serial::write_char('>');
                    return;
                case 0x47: // Home
                    Keyboard::put_buffer((char)KeyCode::KEY_HOME);
                    return;
                case 0x4F: // End
                    Keyboard::put_buffer((char)KeyCode::KEY_END);
                    return;
                case 0x53: // Delete
                    Keyboard::put_buffer((char)KeyCode::KEY_DELETE);
                    Serial::write_char('-');
                    return;
            }
        } else { // Key release
            // Skip key release for extended keys
            return;
        }
    }
    
    // Regular keys
    if (scancode & 0x80) {
        // Key release
        uint8_t key = scancode & 0x7F;
        if (key == 0x2A || key == 0x36) {  // Shift release
            shift_pressed = false;
        } else if (key == 0x1D) {          // Ctrl release
            ctrl_pressed = false;
        }
        return;
    }
    
    // Key press
    if (scancode == 0x2A || scancode == 0x36) {  // Shift press
        shift_pressed = true;
        return;
    }
    
    if (scancode == 0x1D) {  // Ctrl press
        ctrl_pressed = true;
        return;
    }
    
    if (scancode == 0x38) {  // Alt press
        alt_pressed = true;
        return;
    }
    
    if (scancode < 128) {
        // Get character from appropriate map
        char c = shift_pressed ? keyboard_scancode_map_shift[scancode] : keyboard_scancode_map[scancode];
        
        if (c != 0) {
            Keyboard::put_buffer(c);
            
            // Echo to serial for feedback (but not arrow keys already echoed)
            if (c != (char)KeyCode::KEY_UP && c != (char)KeyCode::KEY_DOWN && 
                c != (char)KeyCode::KEY_LEFT && c != (char)KeyCode::KEY_RIGHT) {
                Serial::write_char(c);
            }
        }
    }
}
