#pragma once

// ==========================================
// MoonOS - keyboard.hpp
// PS/2 Keyboard driver (placeholder)
// Akan diisi saat IDT sudah siap
// ==========================================

#include "../include/types.hpp"

namespace Keyboard {

    void init();
    char get_char();    // blocking
    bool has_input();   // cek apakah ada input

} // namespace Keyboard
