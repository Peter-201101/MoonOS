#pragma once

// ==========================================
// MoonOS CLI Shell - Header
// Simple command-line interface (no GUI)
// ==========================================

#include "../include/types.hpp"

namespace Shell {

    // Initialize shell system
    void init();

    // Run shell loop (blocking, main CLI)
    void run();

    // Parse and execute command
    void execute_command(const char* cmd);

    // Print prompt
    void print_prompt();

    // Print welcome message
    void print_welcome();

} // namespace Shell
