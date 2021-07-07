#include "../device/ide.h"
#include "../kernel/ipc.h"
#include "const.h"
#include "debug.h"
#include "../include/stdio.h"

public void * va2la(pid_t proc, void * va);


/*===========================================================================*
 *				task_ide				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1>
 * 所属进程: TASK_IDE
 *****************************************************************************/
public void task_ide()
{// 1
    message_t msg;
    ide_init();

    while(1)
    {
        receive(ANY, &msg);

        int src = msg.source;
        switch(msg.type)
        {
        case DEV_OPEN:
            ide_identify(0);
            break;
        case DEV_READ:
            ide_read(msg.param1, msg.param2, msg.param3, va2la(src, (void *)msg.param4));
            break;
        case DEV_WRITE:
            ide_write(msg.param1, msg.param2, msg.param3, va2la(src, (void *)msg.param4));
            break;
        default:
            panic("{ IDE } unknown msg %d\n", msg.type);
            break;
        }

        send(src, &msg);
    }
}

