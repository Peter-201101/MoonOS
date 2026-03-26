#include "setup.hpp"

#include <drivers/serial.hpp>
#include <utils/io.hpp>
#include <system/config/config.hpp>
#include <utils/string.hpp>

// ==============================
// Internal Input Helper
// ==============================
static void input_field(const char* label, char* buf, int max, bool hidden = false) {
    Serial::write(label);

    int idx = 0;

    while (true) {
        if (IO::inb(0x3F8 + 5) & 0x01) {
            char c = (char)IO::inb(0x3F8);

            // ENTER
            if (c == '\r' || c == '\n') {
                buf[idx] = '\0';
                Serial::write("\r\n");
                return;
            }

            // BACKSPACE
            if ((c == '\b' || c == '\x7f') && idx > 0) {
                idx--;
                Serial::write("\b \b");
            }

            // PRINTABLE CHAR
            else if (c >= 0x20 && idx < max - 1) {
                buf[idx++] = c;

                if (hidden) {
                    Serial::write_char('*'); // mask password
                } else {
                    Serial::write_char(c);
                }
            }
        }
    }
}

// ==============================
// Setup Wizard
// ==============================
void run_setup() {
    Serial::writeln("=================================");
    Serial::writeln("        MoonOS First Setup       ");
    Serial::writeln("=================================");

    char username[32];
    char password[32];
    char hostname[32];
    char timezone[32];

    input_field("Username : ", username, 32);
    input_field("Password : ", password, 32, true);
    input_field("Hostname : ", hostname, 32);
    input_field("Timezone : ", timezone, 32);

    // ==============================
    // Save to Config
    // ==============================
    // ... di dalam run_setup() ...
    SystemConfig cfg;

    String::memcpy(cfg.username, username, 32);
    String::memcpy(cfg.password, password, 32);
    String::memcpy(cfg.hostname, hostname, 32);
    String::memcpy(cfg.timezone, timezone, 32);

    Config::set(cfg);

    // ==============================
    // Debug Output (sementara)
    // ==============================
    Serial::writeln("\n[Setup Complete]");
    Serial::writeln("-----------------------------");

    auto saved = Config::get();

    Serial::write("User     : ");
    Serial::writeln(saved->username);

    Serial::write("Hostname : ");
    Serial::writeln(saved->hostname);

    Serial::write("Timezone : ");
    Serial::writeln(saved->timezone);

    Serial::writeln("-----------------------------");
}