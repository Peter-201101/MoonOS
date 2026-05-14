#ifndef MEMORY_H
#define MEMORY_H

#include <types.h>

void  heap_init(void);
void  heap_dump(void);

void *kmalloc(size_t size);
void  kfree(void *ptr);
void *kcalloc(size_t n, size_t size);
void *krealloc(void *ptr, size_t size);

#endif