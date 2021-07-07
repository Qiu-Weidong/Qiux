#include "tty.h"
#include "../kernel/ipc.h"
#include "const.h"
#include "keyboard.h"
#include "../include/string.h"
#include "../kernel/proc.h"
#include "console.h"
#include "pic.h"

#define TTY_BUFFER_SIZE 256
// 私有类型定义
typedef struct TTY
{
    char input_buf[TTY_BUFFER_SIZE];
    int head;
    int tail;

    int caller; // 请求tty服务的进程
    void * req_buf; // 请求进程的缓冲区
    int tty_trans_cnt;
    int tty_left_cnt;
    console_t console; // tty使用的console
} tty_t;

// 私有变量定义
private tty_t tty_list[8];
private int nr_tty;
private tty_t * cur_tty;

// 私有函数定义
private inline bool_t tty_input_full(tty_t * p_tty)
{
    return (p_tty->tail + 1 ) % TTY_BUFFER_SIZE == p_tty->head;
}
private inline bool_t tty_input_empty(tty_t * p_tty)
{
    return p_tty->head == p_tty->tail;
}
private void tty_dev_read(tty_t * p_tty)
{
    // 从键盘缓冲区读取输入，并将可打印字符放入当前tty的输入缓冲
    while(!kbuf_empty())
    {
        uint16_t key = keyboard_parse();
        if(!(key & FLAG_MAKE)) continue;
        if(key & FLAG_EXT) // 是控制键, 暂时先忽略
        {
            if((int8_t)key == (int8_t)UP)
            {
                console_scroll_up(&p_tty->console, 1);
                console_flush(&p_tty->console);
            }
            else if((int8_t)key == (int8_t)DOWN)
            {
                console_scroll_down(&p_tty->console, 1);
                console_flush(&p_tty->console);
            }
        }
        else {
            // 是可打印字符, 放入tty输入缓冲区
            if(!tty_input_full(p_tty))
            {
                p_tty->input_buf[p_tty->tail++] = key;
                p_tty->tail %= TTY_BUFFER_SIZE;
            }
        }
    }
    
}
private void tty_dev_write(tty_t * p_tty)
{
    // 从tty的输入缓冲中读出可打印字符，并放入进程的输入缓存中(如果有的话)
    while(!tty_input_empty(p_tty))
    {
        char ch = p_tty->input_buf[p_tty->head++];
        p_tty->head %= TTY_BUFFER_SIZE;

        if(p_tty->tty_left_cnt) // 有进程接收数据
        {
            message_t msg;
            if(ch >= ' ' && ch <= '~')
            {
                char * p = p_tty->req_buf + p_tty->tty_trans_cnt;
                *p = ch;
                p_tty->tty_left_cnt--;
                p_tty->tty_trans_cnt++;
                // 回显
                console_putc(&p_tty->console, ch, HIGHLIGHT|FG_WHITE);
                if(p_tty == cur_tty) console_flush(&p_tty->console);
                if(p_tty->tty_left_cnt == 0)
                {
                    msg.param1 = p_tty->tty_trans_cnt;
                    send(p_tty->caller, &msg);
                }
            }
            else if(ch == '\n')
            {
                console_putc(&p_tty->console, ch, HIGHLIGHT|FG_WHITE);
                if(p_tty == cur_tty) console_flush(&p_tty->console);
                msg.param1 = p_tty->tty_trans_cnt;
                send(p_tty->caller, &msg);
            }
            else if(ch == '\b' && p_tty->tty_trans_cnt > 0)
            {
                console_putc(&p_tty->console, ch, HIGHLIGHT|FG_WHITE);
                if(p_tty == cur_tty) console_flush(&p_tty->console);
                p_tty->tty_trans_cnt--;
                p_tty->tty_left_cnt++;
            }
            
        }
    }
}

/*===========================================================================*
 *				tty_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 初始化tty
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_init()
{
    nr_tty = 3; 

    memset(tty_list, 0, sizeof(tty_list));
    console_init(&tty_list[0].console, 0, 0x2000);
    console_init(&tty_list[1].console, 0x2000, 0x3000);
    console_init(&tty_list[2].console, 0x3000, 0x4000);

    tty_switch(0); // 当前终端为0号终端
    kbd_init();
    enable_irq(1);
}

/*===========================================================================*
 *				tty_read				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 从tty中读取数据
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_read(void * buffer, size_t size, pid_t pid)
{
    process_t * proc = pid2proc(pid);
    tty_t * p_tty = &tty_list[proc->tty];

    p_tty->caller = pid;
    p_tty->req_buf = buffer;
    p_tty->tty_left_cnt = size;
    p_tty->tty_trans_cnt = 0;
}

/*===========================================================================*
 *				tty_write				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 向tty中写数据
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_write(void * buffer, size_t size, pid_t pid)
{
    process_t * proc = pid2proc(pid);
    tty_t * p_tty = &tty_list[proc->tty];

    // 直接往console里面写就行了
    for(int i=0;i<size;i++)
        console_putc(&p_tty->console, ((char *)buffer)[i], HIGHLIGHT|FG_WHITE);
    if(p_tty == cur_tty) console_flush(&p_tty->console);
    message_t msg;
    send(pid, &msg);
}

/*===========================================================================*
 *				tty_switch				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 切换tty
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_switch(int idx)
{
    cur_tty = &tty_list[idx];
}

/*===========================================================================*
 *				tty_dev_read_cur				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 将键盘缓冲区中解析到的数据放入当前tty的输入缓冲区中
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_dev_read_cur()
{
    tty_dev_read(cur_tty);
}

/*===========================================================================*
 *				tty_dev_write_all				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 循环每个tty，执行tty_dev_write
 * 执行进程 TASK_TTY
 *****************************************************************************/
public void tty_dev_write_all()
{
    for(int i=0;i<nr_tty;i++) tty_dev_write(&tty_list[i]);
}
