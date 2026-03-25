#include "idt.hpp"
#include <drivers/serial.hpp>
#include <utils/io.hpp> // Pastikan ada outb/inb di sini

static IDT::Entry idt_entries[256];
static IDT::Descriptor idt_descriptor;

extern "C" void idt_flush(uint64_t descriptor_ptr);
extern "C" void irq1_handler(); // Handler dari assembly

// PIC Remapping: Menggeser IRQ hardware agar tidak bentrok dengan Exception CPU
void pic_remap() {
    IO::outb(0x20, 0x11); IO::outb(0xA0, 0x11); // ICW1: Inisialisasi
    IO::outb(0x21, 0x20); IO::outb(0xA1, 0x28); // ICW2: Offset (IRQ0-7 -> 32-39)
    IO::outb(0x21, 0x04); IO::outb(0xA1, 0x02); // ICW3: Cascade
    IO::outb(0x21, 0x01); IO::outb(0xA1, 0x01); // ICW4: 8086 Mode
    
    // Masking: 0xFD aktifkan IRQ1 (Keyboard), 0xFF matikan sisanya
    IO::outb(0x21, 0xFD); 
    IO::outb(0xA1, 0xFF);
}

namespace IDT {
    void set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags) {
        idt_entries[num].offset_low  = handler & 0xFFFF;
        idt_entries[num].offset_mid  = (handler >> 16) & 0xFFFF;
        idt_entries[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
        idt_entries[num].selector    = selector;
        idt_entries[num].ist         = 0;
        idt_entries[num].type_attr   = flags;
        idt_entries[num].zero        = 0;
    }

    void init() {
        idt_descriptor.size   = (sizeof(Entry) * 256) - 1;
        idt_descriptor.offset = (uintptr_t)&idt_entries;

        // 1. Remap PIC dulu
        pic_remap();

        // 2. Set default handler untuk semua
        for (int i = 0; i < 256; i++) {
            set_gate(i, 0, 0x08, 0x8E); // Placeholder
        }

        // 3. Daftarkan Handler Keyboard (IRQ 1 = Int 33)
        set_gate(33, (uint64_t)irq1_handler, 0x08, 0x8E);

        idt_flush((uintptr_t)&idt_descriptor);
        __asm__ volatile ("sti"); // Nyalakan Interupsi!
        Serial::writeln("[IDT] Initialized & Interrupts Enabled");
    }
}