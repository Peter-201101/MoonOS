#pragma once

// ==========================================
// MoonOS - idt.hpp
// Interrupt Descriptor Table
// Handle semua interrupt & exception
// ==========================================

#include "../../../include/types.hpp"

namespace IDT {

    struct Entry {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t  ist;
        uint8_t  type_attr;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t zero;
    } __attribute__((packed));

    struct Descriptor {
        uint16_t size;
        uint64_t offset;
    } __attribute__((packed));

    void init();
    void set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags);

} // namespace IDT
