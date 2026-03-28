#include "allocator.hpp"

namespace Kernel {

uint8_t Allocator::pool[POOL_SIZE];
uint32_t Allocator::used = 0;

void* Allocator::allocate(uint32_t size)
{
    if (used + size > POOL_SIZE) {
        // Out of memory - just return null
        return nullptr;
    }
    
    void* ptr = pool + used;
    used += size;
    return ptr;
}

void Allocator::deallocate(void* ptr)
{
    // Our simple allocator doesn't support freeing individual objects
    // It's a "bump allocator" - only for kernel initialization
    (void)ptr;  // unused
}

}; // namespace Kernel

// Global new/delete operators
void* operator new(unsigned long size) {
    return Kernel::Allocator::allocate((uint32_t)size);
}

void* operator new[](unsigned long size) {
    return Kernel::Allocator::allocate((uint32_t)size);
}

void operator delete(void* ptr) {
    (void)ptr;
}

void operator delete[](void* ptr) {
    (void)ptr;
}

void operator delete(void* ptr, unsigned long size) {
    (void)ptr;
    (void)size;
}

void operator delete[](void* ptr, unsigned long size) {
    (void)ptr;
    (void)size;
}
