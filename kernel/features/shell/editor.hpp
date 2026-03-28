#pragma once

// ==========================================
// MoonOS - CLI Text Editor (nano-like)
// Simple text editor for kernel files
// ==========================================

#include "../../include/types.hpp"

// Max file size in editor
static const uint32_t EDITOR_MAX_FILE_SIZE = 8192;

namespace Editor {

    // Initialize editor
    void init();

    // Open and edit a file
    // Returns true if file was saved
    bool edit_file(const char* filename);

    // Print editor help
    void print_help();

} // namespace Editor
