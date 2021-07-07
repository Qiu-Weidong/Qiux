#include "../include/stdio.h"


int main(int argc, char * argv[])
{
    for(int i=1;i<argc;i++)
    {
        puts(argv[i]);
        putchar(' ');
    }
    putchar('\n');
    return 0;
}

