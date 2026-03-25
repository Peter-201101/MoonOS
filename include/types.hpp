#pragma once

// ==========================================
// MoonOS - types.hpp
// Definisi tipe data dasar untuk seluruh OS
// ==========================================

// Unsigned integers
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

// Signed integers
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

// Size types
typedef uint64_t            size_t;
typedef int64_t             ssize_t;
typedef uint64_t            uintptr_t;

// NULL
#define NULL 0

// Ukuran memory
#define KB(x) ((x) * 1024ULL)
#define MB(x) ((x) * 1024ULL * 1024ULL)
#define GB(x) ((x) * 1024ULL * 1024ULL * 1024ULL)
