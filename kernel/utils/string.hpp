#pragma once

// ==========================================
// MoonOS - string.hpp
// Implementasi string utility sendiri
// Tidak pakai stdlib
// ==========================================

#include "../include/types.hpp"

namespace String {

    size_t length(const char* str);
    int    compare(const char* a, const char* b);
    char*  copy(char* dest, const char* src);
    char*  concat(char* dest, const char* src);
    void   memset(void* ptr, uint8_t val, size_t n);
    void   memcpy(void* dest, const void* src, size_t n);
    int    memcmp(const void* a, const void* b, size_t n);

} // namespace String
