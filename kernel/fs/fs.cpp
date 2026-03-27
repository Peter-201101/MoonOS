#include "fs.hpp"
#include <drivers/disk/ata.hpp>
#include <drivers/serial.hpp>
#include <utils/string.hpp>

static FS::Superblock sblock;

void FS::init() {
    uint8_t temp[1024]; // Buffer untuk 2 sektor
    if (ATA::read_sector(2, temp) && ATA::read_sector(3, temp + 512)) {
        String::memcpy(&sblock, temp, sizeof(FS::Superblock));
        if (sblock.magic == MOONFS_MAGIC) Serial::writeln("[FS] MoonFS Mounted.");
        else Serial::writeln("[FS] Magic Mismatch! Need Format.");
    }
}

void FS::format() {
    String::memset(&sblock, 0, sizeof(FS::Superblock));
    sblock.magic = MOONFS_MAGIC;
    sblock.file_count = 0;
    
    uint8_t* p = (uint8_t*)&sblock;
    if (ATA::write_sector(2, p) && ATA::write_sector(3, p + 512)) {
        Serial::writeln("[FS] Disk Formatted Successfully.");
    }
}

bool FS::list_files() {
    Serial::writeln("\n--- File List ---");
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (sblock.files[i].present) {
            Serial::write("- "); Serial::writeln(sblock.files[i].name);
            found++;
        }
    }
    if (found == 0) Serial::writeln("(Empty)");
    return true;
}