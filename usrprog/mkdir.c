#include "../kernel/syscall.h"
#include "../include/stdio.h"
#include "const.h"


int main(int argc, const char * argv[])
{
    if(argc < 2) return puts("too little arguments!\n"), 1;
    mkdir(argv[1]);
    return 0;
}

