#ifndef STRING_H
#define STRING_H

#include <types.h>

size_t  strlen(const char *s);
char   *strcpy(char *dst, const char *src);
char   *strncpy(char *dst, const char *src, size_t n);
int     strcmp(const char *s1, const char *s2);
int     strncmp(const char *s1, const char *s2, size_t n);
void   *memset(void *dst, int val, size_t n);
void   *memcpy(void *dst, const void *src, size_t n);
int     memcmp(const void *a, const void *b, size_t n);
void   *memmove(void *dst, const void *src, size_t n);
char   *strchr(const char *s, int c);

#endif