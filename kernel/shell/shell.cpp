// ==========================================
// MoonOS CLI Shell - Implementation  
// Simple command-line interface (no GUI)
// ==========================================

#include "shell.hpp"
#include <drivers/serial.hpp>
#include <drivers/keyboard.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include "editor.hpp"
#include <fs/fs.hpp>

#define CMD_BUFFER_SIZE 256
#define CMD_HISTORY_SIZE 32

namespace Shell {

    static char command_buffer[CMD_BUFFER_SIZE];
    static char command_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
    static uint8_t history_index = 0;
    static uint8_t history_count = 0;
    static bool shell_running = true;

    // Forward declarations for built-in commands
    static void cmd_help(int argc, char** argv);
    static void cmd_echo(int argc, char** argv);
    static void cmd_clear(int argc, char** argv);
    static void cmd_version(int argc, char** argv);
    static void cmd_poweroff(int argc, char** argv);
    static void cmd_edit(int argc, char** argv);
    // File system commands
    static void cmd_ls(int argc, char** argv);
    static void cmd_mkdir(int argc, char** argv);
    static void cmd_cd(int argc, char** argv);
    static void cmd_pwd(int argc, char** argv);
    static void cmd_touch(int argc, char** argv);
    static void cmd_rm(int argc, char** argv);
    static void cmd_cat(int argc, char** argv);

    // Built-in command table
    struct Command {
        const char* name;
        void (*func)(int argc, char** argv);
        const char* description;
    };

    static const Command builtin_commands[] = {
        {"help", cmd_help, "Show available commands"},
        {"echo", cmd_echo, "Print text"},
        {"clear", cmd_clear, "Clear screen"},
        {"version", cmd_version, "Show kernel version"},
        {"edit", cmd_edit, "Edit a text file"},
        {"ls", cmd_ls, "List directory contents"},
        {"mkdir", cmd_mkdir, "Create directory"},
        {"cd", cmd_cd, "Change directory"},
        {"pwd", cmd_pwd, "Print working directory"},
        {"touch", cmd_touch, "Create empty file"},
        {"rm", cmd_rm, "Delete file/directory"},
        {"cat", cmd_cat, "Show file contents"},
        {"poweroff", cmd_poweroff, "Shutdown system"},
        {nullptr, nullptr, nullptr} // sentinel
    };

    // Helper: Parse command line into argc/argv
    static void parse_command(char* line, int& argc, char** argv) {
        argc = 0;
        char* current = line;
        
        while (*current != '\0' && argc < 16) {
            // Skip whitespace
            while (*current == ' ' || *current == '\t') current++;
            if (*current == '\0') break;
            
            // Mark start of argument
            argv[argc++] = current;
            
            // Find end of argument
            while (*current != '\0' && *current != ' ' && *current != '\t') {
                current++;
            }
            
            // Null-terminate argument
            if (*current != '\0') {
                *current = '\0';
                current++;
            }
        }
        
        argv[argc] = nullptr;
    }

    // Built-in commands implementation
    static void cmd_help([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        Serial::writeln("=== MoonOS CLI Help ===");
        Serial::writeln("Built-in commands:");
        
        for (const Command* cmd = builtin_commands; cmd->name != nullptr; cmd++) {
            Serial::write("  ");
            Serial::write(cmd->name);
            Serial::write("      - ");
            Serial::writeln(cmd->description);
        }
    }

    static void cmd_echo(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("");
            return;
        }
        
        for (int i = 1; i < argc; i++) {
            Serial::write(argv[i]);
            if (i < argc - 1) Serial::write(" ");
        }
        Serial::writeln("");
    }

    static void cmd_clear([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        // Clear screen with ANSI escape code
        Serial::writeln("\033[2J\033[H");
    }

    static void cmd_version([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        Serial::writeln("MoonOS v0.1.0 CLI Edition");
        Serial::writeln("No GUI, pure CLI interface");
    }

    static void cmd_poweroff([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        Serial::writeln("Powering off...");
        shell_running = false;
        __asm__ volatile("cli; hlt");
    }

    static void cmd_edit(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: edit <filename>");
            return;
        }
        
        Editor::init();
        Editor::edit_file(argv[1]);
    }

    // File System Commands
    
    static void cmd_ls([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        FS::list_files();
    }

    static void cmd_mkdir(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: mkdir <dirname>");
            return;
        }
        FS::mkdir(argv[1]);
    }

    static void cmd_cd(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: cd <dirname>");
            return;
        }
        FS::chdir(argv[1]);
    }

    static void cmd_pwd([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
        static char path_buffer[256];
        if (FS::pwd(path_buffer)) {
            Serial::write("Current directory: ");
            Serial::writeln(path_buffer);
        }
    }

    static void cmd_touch(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: touch <filename>");
            return;
        }
        FS::touch(argv[1]);
    }

    static void cmd_rm(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: rm <filename>");
            return;
        }
        FS::rm(argv[1]);
    }

    static void cmd_cat(int argc, char** argv) {
        if (argc < 2) {
            Serial::writeln("Usage: cat <filename>");
            return;
        }
        FS::cat(argv[1]);
    }

    void print_welcome() {
        Serial::writeln("\n╔════════════════════════════════════════╗");
        Serial::writeln("║     Welcome to MoonOS CLI v0.1.0       ║");
        Serial::writeln("║     No GUI - Pure command line OS      ║");
        Serial::writeln("║     File System Ready!                 ║");
        Serial::writeln("╚════════════════════════════════════════╝");
        Serial::writeln("Type 'help' for available commands.\n");
    }

    void print_prompt() {
        static char dir_name[256];
        FS::pwd(dir_name);
        
        Serial::write("moonos:");
        Serial::write(dir_name);
        Serial::write("> ");
    }

    void execute_command(const char* cmd) {
        if (cmd[0] == '\0') return; // empty command
        
        // Make mutable copy for parsing
        static char cmd_copy[CMD_BUFFER_SIZE];
        String::memcpy(cmd_copy, cmd, CMD_BUFFER_SIZE);
        
        // Parse command
        char* argv[16];
        int argc;
        parse_command(cmd_copy, argc, argv);
        
        if (argc == 0) return;
        
        // Find and execute built-in command
        for (const Command* c = builtin_commands; c->name != nullptr; c++) {
            if (String::compare(argv[0], c->name) == 0) {
                c->func(argc, argv);
                return;
            }
        }
        
        // Unknown command
        Serial::write("Unknown command: '");
        Serial::write(argv[0]);
        Serial::writeln("'. Type 'help' for available commands.");
    }

    void init() {
        Serial::writeln("\n[SHELL] Initializing CLI shell...");
        Keyboard::init();
        history_count = 0;
        history_index = 0;
        shell_running = true;
    }

    void run() {
        print_welcome();
        
        while (shell_running) {
            print_prompt();
            
            // Read command line with history and arrow key support
            uint32_t cmd_len = 0;
            uint32_t cursor_pos = 0;  // Cursor position in command
            int8_t history_offset = -1; // -1 = no history selected
            char c;
            
            while (true) {
                // Try keyboard first, then fall back to serial input
                bool has_input = Keyboard::try_read(c);
                
                if (!has_input) {
                    // Fallback: check serial port for input (works with piped stdin in QEMU)
                    uint8_t status = IO::inb(0x3F8 + 5);
                    if (status & 0x01) {  // Data available on serial
                        c = (char)IO::inb(0x3F8);
                        has_input = true;
                    }
                }
                
                if (has_input) {
                    // Handle special keys
                    if (c == (char)KeyCode::KEY_UP) {
                        // Navigate to previous command in history
                        if (history_offset == -1) {
                            // First press: start from newest
                            if (history_count > 0) {
                                history_offset = 0;
                            }
                        } else if (history_offset < history_count - 1) {
                            history_offset++;
                        }
                        
                        if (history_offset >= 0) {
                            // Load command from history
                            String::memcpy(command_buffer, 
                                         command_history[(history_count - 1 - history_offset) % CMD_HISTORY_SIZE],
                                         CMD_BUFFER_SIZE);
                            cmd_len = String::length(command_buffer);
                            cursor_pos = cmd_len;
                            
                            // Erase current line and reprint
                            for (uint32_t i = 0; i < cursor_pos; i++) Serial::write("\b \b");
                            Serial::write(command_buffer);
                        }
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_DOWN) {
                        // Navigate to next command in history
                        if (history_offset > 0) {
                            history_offset--;
                            String::memcpy(command_buffer,
                                         command_history[(history_count - 1 - history_offset) % CMD_HISTORY_SIZE],
                                         CMD_BUFFER_SIZE);
                            cmd_len = String::length(command_buffer);
                            cursor_pos = cmd_len;
                            
                            // Erase and reprint
                            for (uint32_t i = 0; i < cursor_pos; i++) Serial::write("\b \b");
                            Serial::write(command_buffer);
                        } else if (history_offset == 0) {
                            // Clear back to empty command
                            for (uint32_t i = 0; i < cmd_len; i++) Serial::write("\b \b");
                            command_buffer[0] = '\0';
                            cmd_len = 0;
                            cursor_pos = 0;
                            history_offset = -1;
                        }
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_LEFT) {
                        // Move cursor left
                        if (cursor_pos > 0) {
                            cursor_pos--;
                            Serial::write("\b");
                        }
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_RIGHT) {
                        // Move cursor right
                        if (cursor_pos < cmd_len) {
                            Serial::write_char(command_buffer[cursor_pos]);
                            cursor_pos++;
                        }
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_HOME) {
                        // Move cursor to start
                        for (uint32_t i = 0; i < cursor_pos; i++) Serial::write("\b");
                        cursor_pos = 0;
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_END) {
                        // Move cursor to end
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write_char(command_buffer[i]);
                        }
                        cursor_pos = cmd_len;
                        continue;
                    }
                    
                    // Handle enter key
                    if (c == '\n' || c == '\r') {
                        command_buffer[cmd_len] = '\0';
                        Serial::writeln("");
                        break;
                    }
                    
                    // Handle backspace
                    if (c == '\b' && cursor_pos > 0) {
                        cursor_pos--;
                        cmd_len--;
                        // Shift remaining chars left
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            command_buffer[i] = command_buffer[i + 1];
                        }
                        command_buffer[cmd_len] = '\0';
                        
                        // Erase on screen
                        Serial::write("\b");
                        // Redraw rest of line
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write_char(command_buffer[i]);
                        }
                        Serial::write(" \b");
                        // Move cursor back
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write("\b");
                        }
                        
                        history_offset = -1; // Exit history mode
                        continue;
                    }
                    
                    if (c == (char)KeyCode::KEY_DELETE && cursor_pos < cmd_len) {
                        // Delete character at cursor
                        cmd_len--;
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            command_buffer[i] = command_buffer[i + 1];
                        }
                        command_buffer[cmd_len] = '\0';
                        
                        // Redraw line
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write_char(command_buffer[i]);
                        }
                        Serial::write(" \b");
                        // Move cursor back
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write("\b");
                        }
                        
                        history_offset = -1;
                        continue;
                    }
                    
                    // Regular printable character
                    if (c >= 32 && c < 127 && cmd_len < CMD_BUFFER_SIZE - 1) {
                        // Insert character at cursor
                        for (uint32_t i = cmd_len; i > cursor_pos; i--) {
                            command_buffer[i] = command_buffer[i - 1];
                        }
                        command_buffer[cursor_pos] = c;
                        cmd_len++;
                        command_buffer[cmd_len] = '\0';
                        
                        // Redraw from cursor
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write_char(command_buffer[i]);
                        }
                        
                        // Move cursor back if not at end
                        cursor_pos++;
                        for (uint32_t i = cursor_pos; i < cmd_len; i++) {
                            Serial::write("\b");
                        }
                        
                        history_offset = -1;
                        continue;
                    }
                }
            }
            
            // Execute command
            if (cmd_len > 0) {
                execute_command(command_buffer);
                
                // Add to history
                if (history_count < CMD_HISTORY_SIZE) {
                    String::memcpy(
                        command_history[history_count],
                        command_buffer,
                        CMD_BUFFER_SIZE
                    );
                    history_count++;
                } else {
                    // Rotate history (shift all left, add to end)
                    for (int i = 0; i < CMD_HISTORY_SIZE - 1; i++) {
                        String::memcpy(
                            command_history[i],
                            command_history[i + 1],
                            CMD_BUFFER_SIZE
                        );
                    }
                    String::memcpy(
                        command_history[CMD_HISTORY_SIZE - 1],
                        command_buffer,
                        CMD_BUFFER_SIZE
                    );
                }
            }
        }
    }

} // namespace Shell
