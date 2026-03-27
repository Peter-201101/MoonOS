#include "idt.hpp"
#include <drivers/serial.hpp>
#include <utils/io.hpp>

static IDT::Entry idt_entries[256];
static IDT::Descriptor idt_descriptor;

extern "C" void idt_flush(uintptr_t descriptor_ptr);
extern "C" void irq1_handler();
extern "C" void default_isr_handler(); // Linker sekarang akan menemukan ini di isr.asm

void pic_remap() {
    IO::outb(0x20, 0x11); IO::outb(0xA0, 0x11);
    IO::outb(0x21, 0x20); IO::outb(0xA1, 0x28); // Map IRQ 0-15 ke 32-47
    IO::outb(0x21, 0x04); IO::outb(0xA1, 0x02);
    IO::outb(0x21, 0x01); IO::outb(0xA1, 0x01);
    
    // Masking: Aktifkan hanya Keyboard (IRQ1)
    IO::outb(0x21, 0xFD); // 0xFD = 11111101 (bit 1 aktif)
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

        pic_remap();

        // 1. Semua isi dengan default_isr_handler agar tidak crash jika ada error
        for (int i = 0; i < 256; i++) {
            set_gate(i, (uint64_t)default_isr_handler, 0x08, 0x8E);
        }

        // 2. Override IRQ 1 (Keyboard) ke handler aslinya
        set_gate(33, (uint64_t)irq1_handler, 0x08, 0x8E);

        idt_flush((uintptr_t)&idt_descriptor);
        // STI dilakukan di kernel_main setelah login/setup selesai
    }
}