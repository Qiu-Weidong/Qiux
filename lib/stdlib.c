#include "../include/stdlib.h"
#include "../include/ctype.h"


public char * itoa(char * buffer, unsigned int num, int scale)
{
    char * q = buffer;

    if(num == 0)
    {
        *q++ = '0';
        *q = '\0';
        return buffer;
    }

    char tmp[16];
    char *p = tmp;
    while(num)
    {
        int t = num % scale;
        if(t >= 10) *p++ = t - 10 +'A';
        else *p++ = t + '0';
        num /= scale;
    }
    

    while(p!=tmp) *q++ = *--p;
    *q = '\0';
    return buffer;
}

public int atoi(const char * str, int base)
{
    int ans = 0;
    while(*str)
    {
        int k = isalpha(*str) ? (isupper(*str) ?  *str - 'A' + 10 : *str - 'a' + 10) : *str - '0';
        ans = ans * base + k;
        str++;
    }
    return ans;
}


