#include "kernel.hpp"
// I/O Features
#include <io/serial.hpp>
#include <io/keyboard.hpp>
#include <io/mouse.hpp>
// GUI Features  
#include <gui/graphics/vesa.hpp>
// CPU/Architecture
#include <x86_64/gdt.hpp>
#include <x86_64/idt.hpp>
// Memory Features
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
// Utilities
#include <io.hpp>
#include <string.hpp>
// Shell & UI Features
#include <shell/shell.hpp>
#include <gui/gui_shell.hpp>
// Storage Features
#include <storage/fs.hpp>
#include <storage/ata.hpp>
// System Components
#include <system/setup/setup.hpp>
#include <system/login/login.hpp>
#include <system/config/config.hpp>

// =============== Multiboot Structures ===============

struct MultibootFramebuffer {
    uint32_t type;          // 0=indexed, 1=RGB
    uint32_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t  bpp;
    uint8_t  reserved[3];
} __attribute__((packed));

struct MultibootTag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct MultibootTagFramebuffer {
    struct MultibootTag tag;
    struct MultibootFramebuffer fb;
} __attribute__((packed));

// =============== Helper Functions ===============

Graphics::MultibootFramebuffer* find_framebuffer_tag(uint64_t multiboot_addr)
{
    if (multiboot_addr == 0) return nullptr;
    
    // Multiboot info structure pointer
    uint32_t* info = (uint32_t*)multiboot_addr;
    uint32_t total_size = *info;
    
    // Iterate through tags
    uint8_t* tag_ptr = (uint8_t*)(multiboot_addr + 8);
    uint8_t* end_ptr = (uint8_t*)(multiboot_addr + total_size);
    
    while (tag_ptr < end_ptr) {
        MultibootTag* tag = (MultibootTag*)tag_ptr;
        
        if (tag->type == 0) {  // End tag
            break;
        }
        
        if (tag->type == 8) {  // Framebuffer tag
            MultibootTagFramebuffer* fb_tag = (MultibootTagFramebuffer*)tag;
            return (Graphics::MultibootFramebuffer*)&fb_tag->fb;
        }
        
        // Next tag (aligned to 8 bytes)
        uint32_t next_offset = ((tag->size + 7) / 8) * 8;
        tag_ptr += next_offset;
    }
    
    return nullptr;
}

extern "C" void kernel_main([[maybe_unused]] uint32_t magic, [[maybe_unused]] uint64_t multiboot_addr) {
    __asm__ volatile("cli"); // Disable interrupts during initialization
    
    // 1. Initialize core systems first
    Serial::init();
    Serial::writeln("[KERNEL] MoonOS x86_64 Bootloader (GUI Version)...");
    
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

    // 2. Initialize Graphics (NEW!)
    Graphics::MultibootFramebuffer* mbt_fb = find_framebuffer_tag(multiboot_addr);
    if (mbt_fb && Graphics::g_framebuffer.init_from_multiboot(mbt_fb)) {
        Serial::writeln("[GRAPHICS] VESA framebuffer initialized");
    } else {
        Serial::writeln("[GRAPHICS] WARNING: Could not initialize graphics - falling back to CLI");
    }

    // 3. Initialize ATA Disk
    Serial::writeln("[INIT] Initializing hardware...");
    ATA::init();
    Serial::writeln("[ATA] Disk controller initialized");
    
    // 4. Read Config from LBA 1
    Config::init();
    Serial::writeln("[CONFIG] Configuration loaded");
    SystemConfig* cfg = Config::get();
    
    // 5. Check if config is empty (first boot)
    if (cfg == nullptr || cfg->username[0] == '\0' || cfg->username[0] == (char)0xFF) {
        Serial::writeln("[SYSTEM] First boot detected - starting setup");
        
        // Format filesystem during setup
        FS::init();
        FS::format();
        Serial::writeln("[FS] Filesystem formatted");
        
        // Run setup with serial input
        run_setup();
        
    } else {
        // 6. Config exists - load filesystem (skip login, go straight to shell)
        Serial::writeln("[SYSTEM] System ready - loading filesystem");
        FS::init();
        Serial::writeln("[FS] Filesystem ready");
        
        // For now, skip login and go straight to shell
        // This is better for development/testing
        // TODO: Add optional login with --login flag or menu
        Serial::write("[SYSTEM] Welcome back, ");
        Serial::writeln(cfg->username);
    }
    
    // 7. After setup/login: Initialize keyboard and mouse
    Keyboard::init();
    Serial::writeln("[KEYBOARD] PS/2 Keyboard driver initialized");
    
    Mouse::g_mouse.init();
    Serial::writeln("[MOUSE] PS/2 Mouse driver initialized");
    
    // Enable interrupts for GUI
    __asm__ volatile("sti");
    
    // 8. Start GUI Shell (if graphics available) or CLI Shell (fallback)
    if (Graphics::g_framebuffer.is_initialized()) {
        Serial::writeln("[SHELL] Starting GUI shell...");
        GUI::Shell gui_shell;
        gui_shell.run();
    } else {
        Serial::writeln("[SHELL] Starting CLI shell (graphics unavailable)...");
        Shell::init();
        Shell::run();
    }
}