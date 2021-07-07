#ifndef QIUX_CTYPE_H_
#define QIUX_CTYPE_H_
#include "type.h"


private inline bool_t islower(char c)
{
    return c >= 'a' && c <= 'z';
}
private inline bool_t isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}
private inline bool_t iscntrl(char c)
{
    return c >= 0 && c < 32 || c == 127;
}
private inline bool_t isdigit(char c)
{
    return c >= '0' && c <= '9';
}
private inline bool_t isgraph(char c)
{
    return c > 32 && c < 127;
}
private inline bool_t isprint(char c)
{
    return c >= 32 && c < 127;
}
private inline bool_t isspace(char c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' ||
           c == '\t' || c == '\v';
}
private inline bool_t isascii(char c)
{
    return c >= 0 && c < 128;
}
private inline bool_t isalpha(char c)
{
    return islower(c) || isupper(c);
}
private inline bool_t isalnum(char c)
{
    return isalpha(c) || isdigit(c);
}
private inline bool_t ispunct(char c)
{
    return isprint(c) && !isalnum(c) && !isspace(c);
}
private inline char toascii(char c)
{
    return c & 0x7f;
}
private inline char tolower(char c)
{
    return isupper(c) ? c - 'A' + 'a' : c;
}
private inline char toupper(char c)
{
    return islower(c) ? c - 'a' + 'A' : c;
}

#endif // QIUX_CTYPE_H_