#ifndef QIUX_CONSOLE_H_
#define QIUX_CONSOLE_H_
#include "type.h"


typedef struct CONSOLE
{
    uint32_t original_addr;         // 占用的显存开始地址，不是实际地址，而是字符地址
    uint32_t v_mem_limit;           // 分配的显存能够占用的总的字符数
    uint32_t current_start_addr;    // 当前显示的开始地址
    uint32_t cursor;                // 当前光标位置    
} console_t;

public void console_scroll_down(console_t * , int );

public void console_scroll_up(console_t *, int );

public void console_init(console_t *, uint32_t , uint32_t );

public void console_clear(console_t *);

public void console_putc(console_t *, char, unsigned char );

public void console_flush(console_t *p_console);


#endif // QIUX_CONSOLE_H_

