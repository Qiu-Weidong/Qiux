#include "console.h"
#include "__asm__.h"
#include "string.h"
#include "debug.h"
#include "vga.h"

#define SCREEN_WIDTH  80
#define SCREEN_HEIGHT 25
#define SCREEN_SIZE   2000

/*VGA相关端口*/
#define	V_MEM_BASE	    0xB8000	/* base of color video memory */
#define	V_MEM_SIZE	    0x8000	/* 32K: B8000H -> BFFFFH */

/*===========================================================================*
 *				console_flush				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 控制台刷新
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void console_flush(console_t *p_console)
{
    vga_flush(p_console->cursor, p_console->current_start_addr);
}

/*===========================================================================*
 *				console_scroll_up				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 控制台上滚
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void console_scroll_up(console_t *p_console, int n)
{
    if (p_console->current_start_addr >= p_console->original_addr + SCREEN_WIDTH * n)
        p_console->current_start_addr -= SCREEN_WIDTH * n;
    else
        p_console->current_start_addr = p_console->original_addr;
}

/*===========================================================================*
 *				console_scroll_down				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 控制台下滚
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void console_scroll_down(console_t *p_console, int n)
{
    p_console->current_start_addr += SCREEN_WIDTH * n;
    p_console->current_start_addr = p_console->current_start_addr < p_console->original_addr + p_console->v_mem_limit ? p_console->current_start_addr : p_console->original_addr + p_console->v_mem_limit;
}

/*===========================================================================*
 *				console_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 初始化控制台
 * 执行进程 TASK_TTY
 * 调用函数 tty_init
 *****************************************************************************/
public void console_init(console_t *p_console, uint32_t base, uint32_t limit)
{
    p_console->current_start_addr = p_console->cursor = p_console->original_addr = base;
    p_console->v_mem_limit = limit;
    console_clear(p_console);
}

/*===========================================================================*
 *				console_clear				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 清空控制台
 * 执行进程 TASK_TTY
 * 调用函数 console_init
 *****************************************************************************/
public void console_clear(console_t *p_console)
{
    short v = ' ' | 0xf00;

    int n = p_console->v_mem_limit;
    unsigned short *p = (unsigned short *)(V_MEM_BASE + (p_console->original_addr << 1));
    while (n--)
        *p++ = v;

    p_console->current_start_addr = p_console->original_addr; // 从实际显存地址(current_start_addr-0xb8000)*2处开始显示
    p_console->cursor = p_console->current_start_addr;        // cursor从0xb8000开始算
}

/*===========================================================================*
 *				console_putc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 向控制台输出字符
 * 执行进程 TASK_TTY
 * 调用函数 tty_dev_write、tty_write
 *****************************************************************************/
public void console_putc(console_t *p_console, char c, unsigned char color)
{
    uint16_t *p_mem = (uint16_t *)(V_MEM_BASE + (p_console->cursor << 1)); // 需要打印的位置

    if (c == '\n')
    {
        if (p_console->cursor < p_console->original_addr + p_console->v_mem_limit - SCREEN_WIDTH)
        {
            p_console->cursor = p_console->cursor + SCREEN_WIDTH - p_console->cursor % SCREEN_WIDTH;
        }
    }
    else if (c == '\b')
    {
        if (p_console->cursor > p_console->original_addr)
        {
            p_mem--;
            *p_mem = (' ' | (color << 8));
            p_console->cursor--;
        }
    }
    else if(c != '\0')
    {
        if (p_console->cursor < p_console->original_addr + p_console->v_mem_limit - 1)
        {
            *p_mem = (c | (color << 8));
            p_console->cursor++;
        }
    }

    while (p_console->cursor < p_console->current_start_addr)
        console_scroll_up(p_console, 1);
    while (p_console->cursor - p_console->current_start_addr > SCREEN_SIZE)
        console_scroll_down(p_console, 1);
}


