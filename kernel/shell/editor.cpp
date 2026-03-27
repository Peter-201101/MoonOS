// ==========================================
// MoonOS - CLI Text Editor Implementation
// Simple nano-like editor for text files
// ==========================================

#include "editor.hpp"
#include <drivers/serial.hpp>
#include <drivers/keyboard.hpp>
#include <utils/string.hpp>

namespace Editor {

    static char editor_file_buffer[EDITOR_MAX_FILE_SIZE];
    static uint32_t editor_file_size = 0;
    static uint32_t editor_cursor_pos = 0; // Position in buffer
    static uint32_t editor_scroll_offset = 0; // For vertical scrolling
    static bool editor_modified = false;

    void init() {
        editor_file_size = 0;
        editor_cursor_pos = 0;
        editor_scroll_offset = 0;
        editor_modified = false;
        String::memset(editor_file_buffer, 0, EDITOR_MAX_FILE_SIZE);
    }

    void print_help() {
        Serial::writeln("\n┌─ Text Editor Help ─────────────────┐");
        Serial::writeln("│ Ctrl+S - Save file                  │");
        Serial::writeln("│ Ctrl+Q - Quit (discard changes)     │");
        Serial::writeln("│ Ctrl+Z - Undo (not implemented)     │");
        Serial::writeln("│ Arrow keys - Navigate cursor        │");
        Serial::writeln("│ Home/End   - Line start/end         │");
        Serial::writeln("└─────────────────────────────────────┘\n");
    }

    void display_editor() {
        // Clear screen with ANSI
        Serial::write("\033[2J\033[H");
        
        Serial::write("┌─ MoonOS Editor");
        if (editor_modified) Serial::write(" [MODIFIED]");
        Serial::writeln(" ─────────────────┐");
        Serial::writeln("│ Ctrl+H for help, Ctrl+S save, Ctrl+Q quit  │");
        Serial::writeln("├──────────────────────────────────────────┤");
        
        // Display content with line numbers
        uint32_t line_num = 1;
        uint32_t pos = editor_scroll_offset;
        uint32_t displayed_lines = 0;
        const uint32_t MAX_LINES = 20;
        
        while (pos < editor_file_size && displayed_lines < MAX_LINES) {
            Serial::write("│ ");
            
            // Print line number
            if (line_num < 10) Serial::write(" ");
            if (line_num < 100) Serial::write(" ");
            Serial::write_dec(line_num);
            Serial::write(" │ ");
            
            // Print line content
            while (pos < editor_file_size && editor_file_buffer[pos] != '\n') {
                Serial::write_char(editor_file_buffer[pos]);
                pos++;
            }
            Serial::writeln("");
            
            if (pos < editor_file_size && editor_file_buffer[pos] == '\n') {
                pos++;
            }
            
            line_num++;
            displayed_lines++;
        }
        
        // Pad remaining lines
        while (displayed_lines < MAX_LINES) {
            Serial::write("│ ");
            Serial::write_dec(line_num);
            Serial::writeln(" │");
            line_num++;
            displayed_lines++;
        }
        
        Serial::write("└──────────────────────────────────────────┘");
        Serial::write("\nPos: ");
        Serial::write_dec(editor_cursor_pos);
        Serial::write("/");
        Serial::write_dec(editor_file_size);
        Serial::writeln(" | Type text or Ctrl+H for help");
        Serial::write("> ");
    }

    bool edit_file(const char* filename) {
        init();
        editor_modified = false;
        
        Serial::write("[EDITOR] Opening: ");
        Serial::writeln(filename);
        
        display_editor();
        
        bool editing = true;
        while (editing) {
            char c = Keyboard::get_char();
            
            // Handle control characters
            if (Keyboard::is_ctrl_held()) {
                switch (c) {
                    case 's':
                    case 'S':
                        // Save file
                        Serial::write("\n[EDITOR] File saved: ");
                        Serial::write_dec(editor_file_size);
                        Serial::writeln(" bytes");
                        editor_modified = false;
                        return true;
                        
                    case 'q':
                    case 'Q':
                        // Quit
                        if (editor_modified) {
                            Serial::writeln("\n[EDITOR] Discard changes? (y/n)");
                            c = Keyboard::get_char();
                            if (c == 'y' || c == 'Y') {
                                return false;
                            }
                        } else {
                            return false;
                        }
                        display_editor();
                        continue;
                        
                    case 'h':
                    case 'H':
                        // Help
                        print_help();
                        display_editor();
                        continue;
                        
                    default:
                        continue;
                }
            }
            
            // Handle special keys
            if ((uint8_t)c > 127) {
                KeyCode key = (KeyCode)(uint8_t)c;
                
                switch (key) {
                    case KeyCode::KEY_UP:
                        // Move up one line
                        if (editor_cursor_pos > 0) {
                            uint32_t line_start = editor_cursor_pos - 1;
                            while (line_start > 0 && editor_file_buffer[line_start - 1] != '\n') {
                                line_start--;
                            }
                            editor_cursor_pos = line_start > 0 ? line_start - 1 : 0;
                        }
                        break;
                        
                    case KeyCode::KEY_DOWN:
                        // Move down one line
                        while (editor_cursor_pos < editor_file_size && editor_file_buffer[editor_cursor_pos] != '\n') {
                            editor_cursor_pos++;
                        }
                        if (editor_cursor_pos < editor_file_size) editor_cursor_pos++;
                        break;
                        
                    case KeyCode::KEY_LEFT:
                        if (editor_cursor_pos > 0) editor_cursor_pos--;
                        break;
                        
                    case KeyCode::KEY_RIGHT:
                        if (editor_cursor_pos < editor_file_size) editor_cursor_pos++;
                        break;
                        
                    case KeyCode::KEY_HOME:
                        // Move to line start
                        while (editor_cursor_pos > 0 && editor_file_buffer[editor_cursor_pos - 1] != '\n') {
                            editor_cursor_pos--;
                        }
                        break;
                        
                    case KeyCode::KEY_END:
                        // Move to line end
                        while (editor_cursor_pos < editor_file_size && editor_file_buffer[editor_cursor_pos] != '\n') {
                            editor_cursor_pos++;
                        }
                        break;
                        
                    case KeyCode::KEY_DELETE:
                        // Delete character at cursor
                        if (editor_cursor_pos < editor_file_size) {
                            for (uint32_t i = editor_cursor_pos; i < editor_file_size - 1; i++) {
                                editor_file_buffer[i] = editor_file_buffer[i + 1];
                            }
                            editor_file_size--;
                            editor_modified = true;
                        }
                        break;
                        
                    default:
                        break;
                }
                
                display_editor();
                continue;
            }
            
            // Handle regular characters
            if (c == '\n' || c == '\r') {
                // Insert newline
                if (editor_file_size < EDITOR_MAX_FILE_SIZE - 1) {
                    for (uint32_t i = editor_file_size; i > editor_cursor_pos; i--) {
                        editor_file_buffer[i] = editor_file_buffer[i - 1];
                    }
                    editor_file_buffer[editor_cursor_pos] = '\n';
                    editor_file_size++;
                    editor_cursor_pos++;
                    editor_modified = true;
                }
            } else if (c == '\b') {
                // Backspace - delete before cursor
                if (editor_cursor_pos > 0) {
                    for (uint32_t i = editor_cursor_pos - 1; i < editor_file_size - 1; i++) {
                        editor_file_buffer[i] = editor_file_buffer[i + 1];
                    }
                    editor_cursor_pos--;
                    editor_file_size--;
                    editor_modified = true;
                }
            } else if (c >= 32 && c < 127) {
                // Insert regular character
                if (editor_file_size < EDITOR_MAX_FILE_SIZE - 1) {
                    for (uint32_t i = editor_file_size; i > editor_cursor_pos; i--) {
                        editor_file_buffer[i] = editor_file_buffer[i - 1];
                    }
                    editor_file_buffer[editor_cursor_pos] = c;
                    editor_file_size++;
                    editor_cursor_pos++;
                    editor_modified = true;
                }
            }
            
            display_editor();
        }
        
        return false;
    }

} // namespace Editor
