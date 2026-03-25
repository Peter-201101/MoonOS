// ==========================================
// MoonOS - vmm.cpp
// Virtual Memory Manager
// TODO: Implement 4-level paging
// ==========================================

#include "vmm.hpp"
#include "../drivers/serial.hpp"

namespace VMM {

    void init() {
        // TODO: Setup PML4, PDPT, PD, PT
        Serial::writeln("[VMM] Stub - not yet implemented");
    }

    void map(uint64_t virt, uint64_t phys, uint64_t flags) {
        (void)virt; (void)phys; (void)flags;
        // TODO: Walk page table dan map virt -> phys
    }

    void unmap(uint64_t virt) {
        (void)virt;
        // TODO: Unmap dan TLB flush
    }

} // namespace VMM
