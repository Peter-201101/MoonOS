// ==========================================
// MoonOS - gdt.cpp
// Global Descriptor Table setup
// ==========================================

#include "gdt.hpp"

static GDT::Entry gdt_entries[3];
static GDT::Descriptor gdt_descriptor;

extern "C" void gdt_flush(uint64_t descriptor_ptr);

static void set_entry(int idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[idx].base_low    = (base & 0xFFFF);
    gdt_entries[idx].base_mid    = (base >> 16) & 0xFF;
    gdt_entries[idx].base_high   = (base >> 24) & 0xFF;
    gdt_entries[idx].limit_low   = (limit & 0xFFFF);
    gdt_entries[idx].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[idx].access      = access;
}

namespace GDT {

    void init() {
        gdt_descriptor.size   = (sizeof(Entry) * 3) - 1;
        gdt_descriptor.offset = (uint64_t)&gdt_entries;

        set_entry(0, 0, 0,          0x00, 0x00); // Null segment
        set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // Kernel Code (64-bit)
        set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xAF); // Kernel Data

        gdt_flush((uint64_t)&gdt_descriptor);
    }

} // namespace GDT
