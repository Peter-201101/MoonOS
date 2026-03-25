#pragma once

// ==========================================
// MoonOS - kernel.hpp
// Main kernel header
// ==========================================

#include "../include/types.hpp"

// Multiboot2 magic number
#define MULTIBOOT2_MAGIC 0x36D76289

extern "C" void kernel_main(uint32_t magic, uint64_t multiboot_addr);
