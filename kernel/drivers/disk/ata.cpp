#include "ata.hpp"
#include <utils/io.hpp>
#include <drivers/serial.hpp>

#define ATA_DATA       0x1F0
#define ATA_SECCOUNT   0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7

static void ata_delay() {
    for (int i = 0; i < 4; i++) IO::inb(ATA_STATUS);
}

static void wait_bsy() {
    int timeout = 1000000;
    while ((IO::inb(ATA_STATUS) & 0x80) && timeout--);
}

static bool wait_drq() {
    int timeout = 1000000;
    while (timeout--) {
        uint8_t status = IO::inb(ATA_STATUS);
        if (status & 0x01) return false; // ERR bit
        if (!(status & 0x80) && (status & 0x08)) return true; // !BSY && DRQ
    }
    return false;
}

void ATA::init() {
    IO::outb(ATA_DRIVE, 0xA0); // Pilih Master Drive
    ata_delay();
    uint8_t status = IO::inb(ATA_STATUS);
    if (status == 0xFF) Serial::writeln("[ATA] Error: No Drive.");
    else Serial::writeln("[ATA] Drive Ready.");
}

bool ATA::read_sector(uint32_t lba, uint8_t* buffer) {
    wait_bsy();
    IO::outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    IO::outb(ATA_SECCOUNT, 1);
    IO::outb(ATA_LBA_LOW,  (uint8_t)lba);
    IO::outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    IO::outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    IO::outb(ATA_COMMAND, 0x20); // READ_PIO
    ata_delay();
    if (!wait_drq()) return false;
    for (int i = 0; i < 256; i++) {
        uint16_t data = IO::inw(ATA_DATA);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }
    return true;
}

bool ATA::write_sector(uint32_t lba, const uint8_t* buffer) {
    wait_bsy();
    IO::outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    IO::outb(ATA_SECCOUNT, 1);
    IO::outb(ATA_LBA_LOW,  (uint8_t)lba);
    IO::outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    IO::outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
    IO::outb(ATA_COMMAND, 0x30); // WRITE_PIO
    ata_delay();
    if (!wait_drq()) return false;
    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        IO::outw(ATA_DATA, data);
    }
    IO::outb(ATA_COMMAND, 0xE7); // FLUSH
    wait_bsy();
    return true;
}