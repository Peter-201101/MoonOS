#pragma once

// ==========================================
// MoonOS - pmm.hpp
// Physical Memory Manager
// Ngatur alokasi frame memori fisik
// ==========================================

#include "../include/types.hpp"

namespace PMM {

    static const size_t PAGE_SIZE = 4096; // 4 KB per page

    void  init(uint64_t mem_start, uint64_t mem_size);
    void* alloc_frame();                  // alokasi 1 frame (4KB)
    void  free_frame(void* addr);         // bebaskan frame
    size_t free_frames();                 // jumlah frame bebas
    size_t total_frames();                // total frame

} // namespace PMM
