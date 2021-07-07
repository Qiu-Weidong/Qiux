#include "../kernel/syscall.h"
#include "../include/stdio.h"

char buffer[128];
int main(int argc, char * argv[])
{
    if(argc < 2) {
        puts("too little arguments\n");
        return 1;
    }
    filedesc_t fd = fopen(argv[1], 0);
    int n ;
    while((n = fread(fd, buffer, 127)) != 0)
    {
        buffer[n] = '\0';
        puts(buffer);
    }
    return 0;
}


