#include "config.hpp"
#include <storage/ata.hpp>
#include <string.hpp>
#include <io/serial.hpp>

static SystemConfig config;

void Config::init() {
    uint8_t buf[512];
    // Coba baca config dari Sektor 1 (LBA 1)
    if (ATA::read_sector(1, buf)) {
        String::memcpy(&config, buf, sizeof(SystemConfig));
        
        // Jika username tidak kosong, berarti sudah pernah setup
        if (config.username[0] != '\0' && config.username[0] != (char)0xFF) {
            Serial::writeln("[CONFIG] Profile loaded from disk.");
        } else {
            String::memset(&config, 0, sizeof(SystemConfig));
            Serial::writeln("[CONFIG] No profile found, first setup required.");
        }
    }
}

void Config::save_to_disk() {
    uint8_t buf[512];
    String::memset(buf, 0, 512);
    String::memcpy(buf, &config, sizeof(SystemConfig));

    if (ATA::write_sector(1, buf)) {
        Serial::writeln("[CONFIG] Success: Profile saved to LBA 1.");
    } else {
        Serial::writeln("[CONFIG] Error: Failed to write to disk.");
    }
}

SystemConfig* Config::get() { return &config; }