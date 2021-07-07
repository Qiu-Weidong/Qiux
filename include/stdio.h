#ifndef QIUX_STDIO_H_
#define QIUX_STDIO_H_

#include "type.h"

public size_t printf(const char *, ...) PRINTF_FORMAT(1, 2);
public int scanf(const char *, ...) PRINTF_FORMAT(1, 2);
public int getchar();
public char * gets(char *);
public size_t vsnprintf(char *, size_t, const char *, va_list) PRINTF_FORMAT(3, 0);
public size_t snprintf(char *, size_t, const char *, ...) PRINTF_FORMAT(3, 4);
public int putchar(int c);
public size_t puts(const char *);
public size_t printk(const char *format, ...) PRINTF_FORMAT(1, 2);

#endif // QIUX_STDIO_H_

