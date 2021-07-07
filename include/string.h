#ifndef QIUX_STRING_H_
#define QIUX_STRING_H_
#include "type.h"

public void *memcpy(void *dest, const void *src, size_t size);

public void *memmove(void *dest, const void *src, size_t size);

public int memcmp(const void *a, const void *b, size_t size);

public void *memset(void *dest, int value, size_t size);

public size_t strlen(const char *str);

public size_t strnlen(const char *str, size_t maxlen);

public char *strcpy(char *dest, const char *src);

public char *strncpy(char *dest, const char *src, size_t size);

public char *strcat(char *dest, const char *src);

public char *strncat(char *dest, const char *src, size_t size);

public int strcmp(const char *str1, const char *str2);

public int strncmp(const char *str1, const char *str2, size_t size);

public char *strchr(char *str, char c);

public char *strrchr(char *str, char c);

public char *strnchr(char *str, char c, size_t size);

public char *strstr(char *str, const char *substr);

public char *strtok(char *str, const char *delim, char ** saveptr);

#endif // QIUX_STRING_H_
