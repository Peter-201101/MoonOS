#include "kernel.hpp"
#include <drivers/serial.hpp>
#include <drivers/keyboard.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <shell/shell.hpp>
#include <fs/fs.hpp>
#include <drivers/disk/ata.hpp>
#include <system/setup/setup.hpp>
#include <system/login/login.hpp>
#include <system/config/config.hpp>

extern "C" void kernel_main([[maybe_unused]] uint32_t magic, [[maybe_unused]] uint64_t multiboot_addr) {
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
    
    // Initialize keyboard driver BEFORE login (login needs keyboard input)
    Keyboard::init();
    Serial::writeln("[KEYBOARD] PS/2 Keyboard driver initialized");
    
    // Enable interrupts for login & shell
    __asm__ volatile("sti");
    
    // Load configuration from disk
    Config::init();
    Serial::writeln("[CONFIG] Configuration loaded");

    // SKIP SETUP/LOGIN FOR NOW - go straight to shell
    // (These require serial/keyboard input that's complex in QEMU)
    
    // Start CLI Shell
    Shell::init();
    Shell::run();
}