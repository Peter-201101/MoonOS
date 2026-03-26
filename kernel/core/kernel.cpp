#include "kernel.hpp"
#include <drivers/serial.hpp>
#include <drivers/keyboard.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
#include <utils/io.hpp>
#include <system/setup/setup.hpp>
#include <system/login/login.hpp>
#include <system/config/config.hpp>
#include <fs/fs.hpp>
#include <drivers/disk/ata.hpp>

static bool is_first_boot = true;

extern "C" void kernel_main(uint32_t magic, uint64_t multiboot_addr) {
    (void)multiboot_addr;

    // =============================
    // INIT SERIAL
    // =============================
    Serial::init();
    Serial::writeln("==========================================");
    Serial::writeln("  MoonOS Booting (Codespaces Mode)");
    Serial::writeln("==========================================");

    // =============================
    // MULTIBOOT CHECK
    // =============================
    if (magic != MULTIBOOT2_MAGIC) goto halt;

    // =============================
    // ARCH INIT
    // =============================
    GDT::init();
    IDT::init();

    // Disable interrupt sementara (pakai polling)
    __asm__ volatile ("cli");

    // =============================
    // SYSTEM INIT
    // =============================
    PMM::init(0x1000000, MB(112));
    VMM::init();
    Keyboard::init();
    FS::init();
    Config::init();

    // =============================
    // SETUP & LOGIN
    // =============================
    if (is_first_boot) {
        run_setup();
        is_first_boot = false;
    }

    run_login();

    // =============================
    // ATA TEST (WAJIB DI SINI)
    // =============================
    ATA::init();

    uint8_t buffer[512];

    // isi data
    for (int i = 0; i < 512; i++) buffer[i] = 'X';

    // WRITE
    if (ATA::write_sector(1, buffer)) {
        Serial::writeln("[TEST] Write OK");
    } else {
        Serial::writeln("[TEST] Write FAIL");
    }

    // clear buffer
    for (int i = 0; i < 512; i++) buffer[i] = 0;

    // READ
    if (ATA::read_sector(1, buffer)) {
        Serial::write("[TEST] Read: ");
        for (int i = 0; i < 16; i++) {
            Serial::write_char(buffer[i]);
        }
        Serial::write("\n");
    } else {
        Serial::writeln("[TEST] Read FAIL");
    }

    // =============================
    // SHELL START
    // =============================
    Serial::writeln("\n==========================================");
    Serial::writeln("  MoonOS Shell Active");
    Serial::writeln("==========================================");

    // =============================
    // MAIN LOOP (TERAKHIR!)
    // =============================
    while (true) {
        if (IO::inb(0x3F8 + 5) & 0x01) {
            char c = (char)IO::inb(0x3F8);

            if (c == '\r') {
                Serial::write("\r\n");
            } else if (c == '\b' || c == '\x7f') {
                Serial::write("\b \b");
            } else if (c >= 0x20) {
                Serial::write_char(c);
            }
        }

        __asm__ volatile ("pause");
    }

halt:
    __asm__ volatile ("cli; hlt");
}