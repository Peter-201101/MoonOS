#include "login.hpp"
#include <drivers/serial.hpp>
#include <utils/io.hpp>
#include <system/config/config.hpp>

static void input_field(const char* label, char* buf, int max, bool hidden = false) {
    Serial::write(label);
    int idx = 0;
    while (true) {
        if (IO::inb(0x3F8 + 5) & 0x01) {
            char c = (char)IO::inb(0x3F8);
            if (c == '\r' || c == '\n') {
                buf[idx] = '\0';
                Serial::write("\r\n");
                return;
            }
            if ((c == '\b' || c == '\x7f') && idx > 0) {
                idx--; Serial::write("\b \b");
            }
            else if (c >= 0x20 && idx < max - 1) {
                buf[idx++] = c;
                Serial::write_char(hidden ? '*' : c);
            }
        }
    }
}

static bool str_eq(const char* a, const char* b) {
    for (int i = 0; i < 32; i++) {
        if (a[i] != b[i]) return false;
        if (a[i] == '\0') return true;
    }
    return true;
}

void run_login() {
    auto cfg = Config::get();
    char user[32], pass[32];

    while (true) {
        Serial::writeln("\n=== MoonOS Login ===");
        input_field("Username : ", user, 32);
        input_field("Password : ", pass, 32, true);

        if (str_eq(user, cfg->username) && str_eq(pass, cfg->password)) {
            Serial::write("\n[Login Success] Welcome, ");
            Serial::writeln(cfg->username);
            return;
        }
        Serial::writeln("\n[Login Failed] Try again.");
    }
}