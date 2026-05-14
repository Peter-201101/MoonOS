#include "stdlib.h"
#include <lib/string.h>

int atoi(const char *s) {
    int n = 0, neg = 0;
    if (*s == '-') { neg = 1; s++; }
    while (*s >= '0' && *s <= '9') n = n*10 + (*s++ - '0');
    return neg ? -n : n;
}

uint32_t atou(const char *s) {
    uint32_t n = 0;
    while (*s >= '0' && *s <= '9') n = n*10 + (*s++ - '0');
    return n;
}

/* kmalloc dll → diimplementasi di core/memory.c */