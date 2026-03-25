#include "kernel.hpp"
#include <drivers/serial.hpp>
#include <drivers/keyboard.hpp>
#include <arch/x86_64/gdt.hpp>
#include <arch/x86_64/idt.hpp>
#include <memory/pmm.hpp>
#include <memory/vmm.hpp>
#include <utils/io.hpp>

#define MEM_START 0x1000000
#define MEM_SIZE  MB(112)

extern "C" void kernel_main(uint32_t magic, uint64_t multiboot_addr) {
    (void)multiboot_addr;

    // 1. Init Serial (Wajib untuk Codespaces)
    Serial::init();
    Serial::writeln("==========================================");
    Serial::writeln("  MoonOS Booting (Codespaces Mode)");
    Serial::writeln("==========================================");

    // 2. Validasi Multiboot
    if (magic != MULTIBOOT2_MAGIC) {
        Serial::writeln("[KERNEL] ERROR: Invalid multiboot2 magic!");
        goto halt;
    }

    // 3. Inisialisasi Arsitektur
    GDT::init();
    Serial::writeln("[GDT] Initialized");

    IDT::init(); // Di dalam sini harus sudah ada pic_remap() dan sti
    Serial::writeln("[IDT] Initialized & Interrupts Enabled");

    // 4. Inisialisasi Memory
    PMM::init(MEM_START, MEM_SIZE);
    VMM::init();

    // 5. Inisialisasi Driver
    Keyboard::init();
    Serial::writeln("[Keyboard] Initialized");

    Serial::writeln("==========================================");
    Serial::writeln("  MoonOS Ready!");
    Serial::writeln("  Type anything in this terminal...");
    Serial::writeln("==========================================");

    // 6. Main Loop (Hybrid Input)
    int counter = 0;
    while (true) {
        // 1. Cek apakah ada data masuk dari Serial (Codespaces mode)
        if (IO::inb(0x3F8 + 5) & 0x01) {
            char c = (char)IO::inb(0x3F8);
            Serial::write("User typed: ");
            Serial::write_char(c);
            Serial::writeln("");
        }

        // 2. Heartbeat: Cetak titik setiap beberapa juta siklus
        if (counter++ % 10000000 == 0) {
            Serial::write("."); 
        }

        __asm__ volatile ("pause"); // Biar gak panas-panas amat CPU-nya
    }

halt:
    __asm__ volatile ("cli; hlt");
}