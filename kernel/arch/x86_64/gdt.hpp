#pragma once

// ==========================================
// MoonOS - gdt.hpp
// Global Descriptor Table
// Ngatur segmentasi memori x86_64
// ==========================================

#include "../../../include/types.hpp"

namespace GDT {

    struct Entry {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t  base_mid;
        uint8_t  access;
        uint8_t  granularity;
        uint8_t  base_high;
    } __attribute__((packed));

    struct Descriptor {
        uint16_t size;
        uint64_t offset;
    } __attribute__((packed));

    void init();

} // namespace GDT
