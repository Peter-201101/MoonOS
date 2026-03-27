#include "kernel.hpp"
#include <drivers/serial.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <fs/fs.hpp>
#include <drivers/disk/ata.hpp>
#include <system/setup/setup.hpp>
#include <system/login/login.hpp>
#include <system/config/config.hpp>
#include <drivers/serial.hpp>

extern "C" void kernel_main(uint32_t magic, uint64_t multiboot_addr) {
    __asm__ volatile("cli"); // Disable interrupts during initialization
    
    // Initialize core systems first - these must succeed or we halt
    Serial::init();
    Serial::writeln("[KERNEL] MoonOS x86_64 Bootloader...");
    
    // Setup CPU tables (GDT & IDT)
    GDT::init();
    Serial::writeln("[GDT] Global Descriptor Table loaded");
    
    IDT::init();
    Serial::writeln("[IDT] Interrupt Descriptor Table loaded");
    
    // Memory management
    PMM::init(0x1000000, 112 * 1024 * 1024);
    Serial::writeln("[PMM] Physical Memory Manager initialized");
    
    VMM::init();
    Serial::writeln("[VMM] Virtual Memory Manager initialized");

    // Hardware & Storage initialization
    Serial::writeln("[INIT] Initializing hardware...");
    
    ATA::init();
    Serial::writeln("[ATA] Disk controller initialized");
    
    FS::init();
    Serial::writeln("[FS] Filesystem ready");
    
    // Load configuration from disk
    Config::init();
    Serial::writeln("[CONFIG] Configuration loaded");

    // System setup or login
    SystemConfig* cfg = Config::get();
    if (cfg == nullptr || cfg->username[0] == '\0' || cfg->username[0] == (char)0xFF) {
        Serial::writeln("[SYSTEM] First boot detected - starting setup");
        run_setup();
    } else {
        Serial::write("[SYSTEM] Welcome back, ");
        Serial::writeln(cfg->username);
    }

    Serial::writeln("[SYSTEM] Starting login sequence...");
    run_login();

    // Enable interrupts for shell mode
    __asm__ volatile("sti");
    
    // Shell Mode
    Serial::writeln("\n[MOON-SHELL] Type 'help' for commands.");
    Serial::write("> ");
    
    char cmd[128];
    int idx = 0;
    while (true) {
        if (IO::inb(0x3F8 + 5) & 0x01) {
            char c = (char)IO::inb(0x3F8);
            if (c == '\r') {
                Serial::write("\r\n");
                cmd[idx] = '\0';
                if (String::compare(cmd, "ls") == 0) FS::list_files();
                else if (String::compare(cmd, "format") == 0) FS::format();
                else if (String::compare(cmd, "clear") == 0) Serial::write("\033[2J\033[H");
                else if (idx > 0) Serial::writeln("Unknown Command.");
                idx = 0; Serial::write("> ");
            } else if (c == '\b' || c == '\x7f') {
                if (idx > 0) { idx--; Serial::write("\b \b"); }
            } else if (idx < 127 && c >= 32) {
                cmd[idx++] = c; Serial::write_char(c);
            }
        }
    }
}