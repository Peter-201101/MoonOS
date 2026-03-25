// ==========================================
// MoonOS - pmm.cpp
// Physical Memory Manager (bitmap allocator)
// ==========================================

#include "pmm.hpp"
#include "../drivers/serial.hpp"

static uint8_t* bitmap    = nullptr;
static size_t   total_mem = 0;
static size_t   used_mem  = 0;
static size_t   n_frames  = 0;

static inline void bitmap_set(size_t idx) {
    bitmap[idx / 8] |= (1 << (idx % 8));
}

static inline void bitmap_clear(size_t idx) {
    bitmap[idx / 8] &= ~(1 << (idx % 8));
}

static inline bool bitmap_test(size_t idx) {
    return bitmap[idx / 8] & (1 << (idx % 8));
}

namespace PMM {

    void init(uint64_t mem_start, uint64_t mem_size) {
        n_frames   = mem_size / PAGE_SIZE;
        total_mem  = mem_size;
        bitmap     = (uint8_t*)mem_start;

        // Mark semua sebagai used dulu
        for (size_t i = 0; i < n_frames / 8; i++) bitmap[i] = 0xFF;

        // Bebaskan frame setelah bitmap
        size_t bitmap_size = n_frames / 8;
        size_t start_frame = (mem_start + bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;

        for (size_t i = start_frame; i < n_frames; i++) {
            bitmap_clear(i);
        }

        used_mem = start_frame * PAGE_SIZE;

        Serial::write("[PMM] Initialized: ");
        Serial::write_dec(n_frames);
        Serial::writeln(" frames");
    }

    void* alloc_frame() {
        for (size_t i = 0; i < n_frames; i++) {
            if (!bitmap_test(i)) {
                bitmap_set(i);
                used_mem += PAGE_SIZE;
                return (void*)(i * PAGE_SIZE);
            }
        }
        Serial::writeln("[PMM] ERROR: Out of memory!");
        return nullptr;
    }

    void free_frame(void* addr) {
        size_t idx = (uint64_t)addr / PAGE_SIZE;
        bitmap_clear(idx);
        used_mem -= PAGE_SIZE;
    }

    size_t free_frames()  { return (total_mem - used_mem) / PAGE_SIZE; }
    size_t total_frames() { return n_frames; }

} // namespace PMM
