#include "../include/stdio.h"
#include "../kernel/syscall.h"


int main(int argc, char * argv[])
{
    if(argc < 2) return puts("too little arguments!\n"), 1;
    fcreate(argv[1]);
    return 0;
}



