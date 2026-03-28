#pragma once

// ==========================================
// MoonOS - vmm.hpp
// Virtual Memory Manager
// Paging setup x86_64 (4-level paging)
// NOTE: Stub, isi setelah PMM stabil
// ==========================================

#include "../include/types.hpp"

namespace VMM {

    void init();
    void map(uint64_t virt, uint64_t phys, uint64_t flags);
    void unmap(uint64_t virt);

    // Page flags
    static const uint64_t FLAG_PRESENT   = (1 << 0);
    static const uint64_t FLAG_WRITABLE  = (1 << 1);
    static const uint64_t FLAG_USER      = (1 << 2);

} // namespace VMM
