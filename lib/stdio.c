#include "../include/stdio.h"
#include "string.h"
#include "stdlib.h"
#include "const.h"
#include "../device/console.h"
#include "../device/tty.h"
#include "__asm__.h"


public size_t fwrite(filedesc_t fd, const char * buffer, size_t n);
public size_t fread(filedesc_t fd, const char * buffer, size_t n);
public size_t printf(const char *format, ...)
{
    size_t size;
    char buf[256];
    va_list args;
    va_start(args, format);
    size = vsnprintf(buf, 256, format, args);
    va_end(args);
    size = fwrite(1, buf, size);
    
    return size;
}

public int scanf(const char * format, ...)
{
    int result = 0;
    char buffer[256];
    va_list args;
    va_start(args, format);

}

public char * gets(char * buffer)
{
    int n = fread(0, buffer, 128);
    buffer[n] = '\0';
    return buffer;
}

public int getchar()
{
    int result = 0;
    fread(0, (const char *)&result, 1);
    return result;
}

public size_t vsnprintf(char *buffer, size_t size, const char *format, va_list list)
{
    char *p = buffer;
    va_list next_arg = list;
    union
    {
        signed char s8;
        signed short s16;
        signed int s32;
        signed long long s64;
        unsigned char u8;
        unsigned short u16;
        unsigned int u32;
        unsigned long long u64;
        float f32;
        double f64;
        char *ptr;
    } type;

    char c;
    size_t count = 0;
    memset(buffer, '\0', size);
    while ((c = *format++) != '\0' && count < size)
    {
        if (c != '%')
        {
            *buffer++ = c;
            count++;
            continue;
        }
        c = *format++;
        switch (c)
        {
        case '%':
            *buffer++ = c;
            count++;
            break;
        case 'p':
        case 'x':
            type.u32 = va_arg(next_arg, int);
            itoa(buffer, type.u32, 16);
            while (*buffer && count < size)
                buffer++, count++;
            break;
        case 'd':
            type.s32 = va_arg(next_arg, int);
            itoa(buffer, type.u32, 10);
            while (*buffer && count < size)
                buffer++, count++;
            break;
        case 'o':
            type.s32 = va_arg(next_arg, int);
            itoa(buffer, type.s32, 8);
            while (*buffer && count < size)
                buffer++, count++;
            break;
        case 'c':
            type.s8 = va_arg(next_arg, int);
            *buffer++ = type.s8;
            count++;
            break;
        case 's':
            type.ptr = va_arg(next_arg, char *);
            strcpy(buffer, type.ptr);
            while (*buffer && count < size)
                buffer++, count++;
            break;
        case 'f':
            break;
        default:
            break;
        }
    }

    *buffer = '\0';
    return count;
}

public size_t snprintf(char *buffer, size_t size, const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    size = vsnprintf(buf, size, fmt, args);
    va_end(args);
    size = fwrite(1, buf, size);
    return size;
}

public int putchar(int c)
{
    char c2 = c;
    fwrite(1, &c2, 1);
    return c;
}

public size_t puts(const char * msg)
{
    return fwrite(1, msg, strlen(msg));
}

public size_t printk(const char *format, ...) // 在内核模式下使用这个，(assert和panic也使用它来打印)
{
    static int pos = 0;


    size_t size;
    char buf[256];
    va_list args;
    va_start(args, format);
    size = vsnprintf(buf, 256, format, args);
    va_end(args);

    uint16_t * p = (uint16_t *)0xb8000 + pos;
    for(int i=0;i<size;i++)
    {
        *p++ = ((FG_RED | HIGHLIGHT) << 8 ) | buf[i];
    }
    pos += size+1;
    return size;
}
