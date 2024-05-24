#ifndef _KERN_LIB_STRING_H_
#define _KERN_LIB_STRING_H_

#ifdef _KERN_

#include "types.h"

void *memset(void *dst, int c, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
void *memzero(void *dst, size_t len);
int memcmp(const void *v1, const void *v2, size_t n);
int strcmp(const char *p, const char *q);
int strncmp(const char *p, const char *q, size_t n);
int strnlen(const char *s, size_t size);
char *strchr(const char *s, char c);
char *strncpy(char *s, const char *t, int n);

#endif  /* _KERN_ */

#endif  /* !_KERN_LIB_STRING_H_ */
