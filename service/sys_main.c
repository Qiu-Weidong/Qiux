#include "type.h"
#include "debug.h"
#include "../kernel/ipc.h"
#include "../device/shutdown.h"
#include "const.h"
#include "../kernel/proc.h"
#include "../device/clock.h"
#include "../include/stdio.h"

/*===========================================================================*
 *				task_sys				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1>
 * 所属进程: TASK_SYS
 *****************************************************************************/
public void task_sys()
{
    message_t msg;
    clock_init();
    
    puts("{ SYS } task sys works.\n");
    while(1)
    {
        receive(ANY, &msg);
        int src = msg.source;

        switch(msg.type)
        {
        case HALT:
            shutdown_poweroff();
            break;
        case REBOOT:
            shutdown_reboot();
            break;
        case EXEC:
        {
            void ** p = (void **)va2la(src, (void *)msg.param2);
            while(*p != nullptr)
            {
                *p = va2la(src, *p);
                p++;
            }
            msg.param1 = process_execute((const char *)va2la(src, (void *)msg.param1), 
                (const char **)va2la(src, (void *)msg.param2), src);
            break;
        }
            
        case EXIT:
            process_exit(src, msg.param1);
            continue;                        // 这里不需要send，因此直接continue
        case WAIT:
            process_wait(msg.param1, src);
            continue;
        default:
            panic("{ SYS } unknown msg type %d", msg.type);
            break;
        }

        send(src, &msg);
    }
}
