#ifndef STDLIB_H
#define STDLIB_H
#include <types.h>

/* Kernel heap — implemented di core/memory.c */
void *kmalloc(size_t size);
void  kfree(void *ptr);
void *kcalloc(size_t n, size_t size);
void *krealloc(void *ptr, size_t size);

/* Konversi */
int      atoi(const char *s);
uint32_t atou(const char *s);

#endif