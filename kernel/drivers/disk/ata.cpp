#include "ata.hpp"
#include <utils/io.hpp>
#include <drivers/serial.hpp>

// ==============================
// ATA PORT
// ==============================
#define ATA_DATA       0x1F0
#define ATA_SECCOUNT   0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7

// ==============================
// COMMAND
// ==============================
#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30
#define ATA_CMD_FLUSH  0xE7

// ==============================
// STATUS FLAG
// ==============================
#define ATA_SR_BSY     0x80
#define ATA_SR_DRQ     0x08
#define ATA_SR_ERR     0x01

// ==============================
// WAIT NOT BUSY
// ==============================
static void wait_bsy() {
    int timeout = 1000000;

    while ((IO::inb(ATA_STATUS) & ATA_SR_BSY) && timeout--);

    if (timeout <= 0) {
        Serial::writeln("[ATA] BSY timeout");
    }
}

// ==============================
// WAIT DRQ READY
// ==============================
static bool wait_drq() {
    int timeout = 1000000;

    while (timeout--) {
        uint8_t status = IO::inb(ATA_STATUS);

        if (status & ATA_SR_ERR) {
            Serial::writeln("[ATA] ERR bit set");
            return false;
        }

        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) {
            return true;
        }
    }

    Serial::writeln("[ATA] DRQ timeout");
    return false;
}

// ==============================
// READ SECTOR (PIO)
// ==============================
bool ATA::read_sector(uint32_t lba, uint8_t* buffer) {
    wait_bsy();

    IO::outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    IO::outb(ATA_SECCOUNT, 1);

    IO::outb(ATA_LBA_LOW,  (uint8_t)(lba));
    IO::outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    IO::outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    IO::outb(ATA_COMMAND, ATA_CMD_READ);

    // 🔥 sinkronisasi status awal
    IO::inb(ATA_STATUS);

    // delay kecil
    for (int i = 0; i < 1000; i++) IO::inb(ATA_STATUS);

    if (!wait_drq()) return false;

    for (int i = 0; i < 256; i++) {
        uint16_t data = IO::inw(ATA_DATA);
        buffer[i * 2]     = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }

    return true;
}

// ==============================
// WRITE SECTOR (PIO)
// ==============================
bool ATA::write_sector(uint32_t lba, const uint8_t* buffer) {
    wait_bsy();

    IO::outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    IO::outb(ATA_SECCOUNT, 1);

    IO::outb(ATA_LBA_LOW,  (uint8_t)(lba));
    IO::outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    IO::outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    IO::outb(ATA_COMMAND, ATA_CMD_WRITE);

    // delay kecil
    for (int i = 0; i < 1000; i++) IO::inb(ATA_STATUS);

    if (!wait_drq()) return false;

    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        IO::outw(ATA_DATA, data);
    }

    // 🔥 WAJIB: flush cache
    IO::outb(ATA_COMMAND, ATA_CMD_FLUSH);
    wait_bsy();

    return true;
}

// ==============================
// INIT
// ==============================
void ATA::init() {
    Serial::writeln("[ATA] Driver initialized");
}