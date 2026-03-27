#include "setup.hpp"
#include <drivers/serial.hpp>
#include <system/config/config.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <drivers/disk/ata.hpp>

// Hapus 'static' agar bisa dipanggil login.cpp
void input_field(const char* label, char* buf, int max, bool hidden) {
    Serial::write(label);
    int idx = 0;
    
    while (true) {
        // Poll serial port for input (works with QEMU -serial stdio)
        uint8_t status = IO::inb(0x3F8 + 5);
        if (status & 0x01) {  // Data available
            char c = (char)IO::inb(0x3F8);
            
            // Handle backspace
            if ((c == '\b' || c == 0x7F) && idx > 0) {
                idx--;
                Serial::write("\b \b");
            }
            // Handle enter/return
            else if (c == '\r' || c == '\n') {
                buf[idx] = '\0';
                Serial::write("\r\n");
                return;
            }
            // Handle regular printable characters
            else if (c >= 0x20 && c < 0x7F && idx < max - 1) {
                buf[idx++] = c;
                Serial::write_char(hidden ? '*' : c);
            }
        }
    }
}

void run_setup() {
    Serial::writeln("\n=================================");
    Serial::writeln("        MoonOS First Setup       ");
    Serial::writeln("=================================");

    char u[32], p[32], h[32], t[32];
    input_field("Username : ", u, 32);
    input_field("Password : ", p, 32, true);
    input_field("Hostname : ", h, 32);
    input_field("Timezone : ", t, 32);

    SystemConfig* cfg = Config::get();
    String::memcpy(cfg->username, u, 32);
    String::memcpy(cfg->password, p, 32);
    String::memcpy(cfg->hostname, h, 32);
    String::memcpy(cfg->timezone, t, 32);

    // SIMPAN PERMANEN
    Config::save_to_disk();
    Serial::writeln("[SETUP] Finished. Your data is now persistent.");
}