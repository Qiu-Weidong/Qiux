#include "../include/string.h"

public void *memcpy(void *dest, const void *src, size_t size)
{
    unsigned char * _dst = dest;
    while(size--)
        *_dst++ = *(unsigned char *)src++;
    return dest;
}

public void *memmove(void *dest, const void *src, size_t size)
{
    if(dest == src) return dest;
    if(dest > src)
    {
        dest += size;
        src  += size;
        while(size--) *(char *)(--dest) = *(char *)(--src);
    }
    else {
        while(size--) *(char *)dest++ = *(char *)src++;
    }
    return dest;
}
public int memcmp(const void *a, const void *b, size_t size)
{
    while(--size && *(char *)a == *(char *)b)
    {
        a++;
        b++;
    }
    return *(char *)a - *(char *)b;
}

// 将dest用value的最低位填充size个字节
public void *memset(void *dest, int value, size_t size)
{
    char * _dst = dest;
    while(size--)
        *_dst++ = (char)value;
    return dest;
}

public size_t strlen(const char *str)
{
    size_t size = 0;
    while(*str++) size++;
    return size;
}

public size_t strnlen(const char * str, size_t maxlen)
{
    size_t ans = 0;
    while(*str++ && maxlen--) ans++;
    return ans;
}


public char *strcpy(char *dest, const char *src)
{
    while(*src) *dest++ = *src++;
    *dest = '\0';
    return dest;
}


public char *strncpy(char *dest, const char * src, size_t size)
{
    while(*src && size--) *dest++ = *src++;
    return dest;
}


public char *strcat(char *dest, const char *src)
{
    char * ret = dest;
    while(*dest) dest++;
    while(*src) *dest++ = *src++;
    return ret;
}

public char *strncat(char *dest, const char *src, size_t size)
{
    char * ret = dest;
    while(*dest) dest++;
    while(*src && size--) *dest++ = *src++;
    return ret;
}

public int strcmp(const char *str1, const char *str2)
{
    while(*str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

public int strncmp(const char *str1, const char *str2, size_t size)
{
    while(--size && *str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

public char *strchr(char *str, char c)
{
    while(*str && *str != c) str++;
    if(*str) return str;
    return nullptr;
}

public char *strrchr(char *str, char c)
{
    char * ret = nullptr;
    while(*str)
    {
        if(*str == c) ret = str;
    }
    return ret;
}

public char *strnchr(char *str, char c, size_t size)
{
    while(size-- && *str && *str != c) str++;
    if(*str) return str;
    return nullptr;
}

public char *strstr(char *str, const char *substr)
{
    while(*str)
    {
        if(strcmp(str, substr) == 0) return str;
    }
    return nullptr;
}

public char *strtok(char *str, const char *delim, char ** saveptr)
{
    if(str == nullptr) str = *saveptr;
    int n = strlen(delim);

    char * s = strstr(str, delim);
    if(s == nullptr) return nullptr;
    *s = '\0';
    *saveptr = s+n;
    return s+n;
}

