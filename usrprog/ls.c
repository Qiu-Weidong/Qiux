#include "../kernel/syscall.h"
#include "../filesys/directory.h"
#include "../include/stdio.h"

directory dir;
int main(int argc, char * argv[])
{
    filedesc_t fd = fopen("/", 0);
    while(fread(fd, &dir, sizeof(directory)) != 0)
    {
        if(dir.ino == 0) continue; 
        puts(dir.name);
        putchar(' ');
    }
    putchar('\n');
    return 0;
}

