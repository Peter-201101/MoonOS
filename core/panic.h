#ifndef PANIC_H
#define PANIC_H

#include <types.h>

void panic(const char *msg, const char *file, uint32_t line);

/* Macro supaya file + line otomatis terisi */
#define PANIC(msg) panic((msg), __FILE__, __LINE__)

/* Assert: kalau kondisi false, panic */
#define ASSERT(cond) \
    do { if (!(cond)) PANIC("Assertion failed: " #cond); } while(0)

#endif