#pragma once

// ==========================================
// MoonOS - keyboard.hpp
// PS/2 Keyboard driver with extended support
// ==========================================

#include "../include/types.hpp"

// Special key codes
enum class KeyCode : uint8_t {
    KEY_ESC     = 0x01,
    KEY_ENTER   = 0x0D,
    KEY_TAB     = 0x09,
    KEY_BACKSP  = 0x08,
    KEY_UP      = 0x80,
    KEY_DOWN    = 0x81,
    KEY_LEFT    = 0x82,
    KEY_RIGHT   = 0x83,
    KEY_DELETE  = 0x84,
    KEY_HOME    = 0x85,
    KEY_END     = 0x86,
    KEY_SHIFT   = 0x87,
    KEY_CTRL    = 0x88,
    KEY_ALT     = 0x89,
    KEY_NULL    = 0x00
};

namespace Keyboard {

    void init();
    char get_char();        // blocking read, returns KeyCode for special keys
    bool has_input();       // check if input available
    bool try_read(char& c); // non-blocking read
    void put_buffer(char c); // internal: add to input buffer
    bool is_shift_held();   // check if shift is pressed
    bool is_ctrl_held();    // check if ctrl is pressed

} // namespace Keyboard
