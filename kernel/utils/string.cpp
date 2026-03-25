// ==========================================
// MoonOS - string.cpp
// ==========================================

#include "string.hpp"

namespace String {

    size_t length(const char* str) {
        size_t i = 0;
        while (str[i]) i++;
        return i;
    }

    int compare(const char* a, const char* b) {
        while (*a && (*a == *b)) { a++; b++; }
        return *(uint8_t*)a - *(uint8_t*)b;
    }

    char* copy(char* dest, const char* src) {
        char* d = dest;
        while ((*d++ = *src++));
        return dest;
    }

    char* concat(char* dest, const char* src) {
        char* d = dest + length(dest);
        while ((*d++ = *src++));
        return dest;
    }

    void memset(void* ptr, uint8_t val, size_t n) {
        uint8_t* p = (uint8_t*)ptr;
        for (size_t i = 0; i < n; i++) p[i] = val;
    }

    void memcpy(void* dest, const void* src, size_t n) {
        uint8_t* d = (uint8_t*)dest;
        const uint8_t* s = (const uint8_t*)src;
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    }

    int memcmp(const void* a, const void* b, size_t n) {
        const uint8_t* p = (const uint8_t*)a;
        const uint8_t* q = (const uint8_t*)b;
        for (size_t i = 0; i < n; i++) {
            if (p[i] != q[i]) return p[i] - q[i];
        }
        return 0;
    }

} // namespace String
