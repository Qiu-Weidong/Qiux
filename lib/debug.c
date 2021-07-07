#include "debug.h"
#include "const.h"
#include "__asm__.h"
#include "../include/stdio.h"

private inline void spin()
{
    for (;;)
        ;
}

public void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    printk("%c  assert(%s) failed: file: %s, base_file: %s, ln%d",
           MAG_CH_SMILE,
           exp, file, base_file, line);
    debug_backtrace();
    spin();
    ud2();
}

public void debug_panic(const char *file, int line, const char *function,
                 const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);

    printk("%c >>panic<< %c at %s:%d in %s(): %s\n", 
        MAG_CH_SPADES, MAG_CH_CLUB, 
        file, line, function, buffer);
    debug_backtrace();
    spin();
    ud2();
}

public void debug_backtrace(void)
{
    void **frame;

    printk("Call stack: %p", __builtin_return_address(0));
    for (frame = __builtin_frame_address(1);(uint32_t)frame >= 0x1000 && frame[0] != NULL;frame = frame[0])
        printk(" %p", frame[1]);
    printk(".\n");

}
