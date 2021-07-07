#include "../device/tty.h"
#include "../kernel/ipc.h"
#include "const.h"
#include "debug.h"

public void * va2la(pid_t proc, void * va);

/*===========================================================================*
 *				task_tty				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1>
 * 所属进程: TASK_TTY
 *****************************************************************************/
public void task_tty()
{
    message_t msg;
    tty_init();

    while(1)
    {
        tty_dev_read_cur();
        tty_dev_write_all();

        receive(ANY, &msg);
        int src = msg.source;

        switch(msg.type)
        {
        case READ:
            tty_read(va2la(src, (void *)msg.param2), msg.param3, src);
            break;
        case WRITE:
            tty_write(va2la(src, (void *)msg.param2), msg.param3, src);
            break;
        case DEV_SELECT:
            tty_switch(0);
            break;
        case HARD_INT:
            break;
        default:
            panic("{ TTY } unknown msg %d\n", msg.type);
            break;
        }
    }
}
