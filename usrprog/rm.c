#include "../kernel/syscall.h"
#include "../include/stdio.h"


int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        puts("too little arguments\n");
        return 1;
    }
    // puts(argv[1]);
    fremove(argv[1]);
    return 0;
}
