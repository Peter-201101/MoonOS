#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <types.hpp>

// Simple kernel memory allocator (no dynamic allocation)
namespace Kernel {

class Allocator {
private:
    static const uint32_t POOL_SIZE = 32 * 1024;  // 32 KB pool
    static uint8_t pool[POOL_SIZE];
    static uint32_t used;
    
public:
    static void* allocate(uint32_t size);
    static void deallocate(void* ptr);
    static uint32_t get_used() { return used; }
};

}; // namespace Kernel

// Global operators for kernel (defined in allocator.cpp)
void* operator new(unsigned long size);
void* operator new[](unsigned long size);
void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, unsigned long size);
void operator delete[](void* ptr, unsigned long size);

#endif // ALLOCATOR_HPP
