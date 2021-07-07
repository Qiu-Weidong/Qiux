#include "../filesys/cache.h"
#include "../kernel/ipc.h"
#include "const.h"
#include "../filesys/super.h"
#include "../filesys/file.h"
#include "../filesys/inode.h"
#include "../filesys/filesys.h"
#include "debug.h"
#include "../include/stdio.h"


public void * va2la(pid_t proc, void * va);
extern super_block super;


/*===========================================================================*
 *				task_fs				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1>
 * 所属进程: TASK_FS
 *****************************************************************************/
public void task_fs()
{// 2
    cache_init();
    inode_init();
    file_init();
    message_t msg;
    
    puts("{ FS } task fs works.\n");
    // msg.type = DEV_OPEN;
    // if(send(TASK_IDE, &msg) == SEND_SUCCESS)
    //     receive(TASK_IDE, &msg);

    cache_read(0, 1, sizeof(super_block), 0, &super);


    while(1)
    {
        receive(ANY, &msg);
        int src = msg.source;

        switch(msg.type)
        {
        case OPEN:
            msg.param1 = filesys_open((const char *)va2la(src, (void *)msg.param1), msg.param2, msg.source);
            break;
        case CLOSE:
            msg.param1 = filesys_close(msg.param1, msg.source);
            break;
        case READ:
            msg.param1 = filesys_read(msg.param1, va2la(src, (void *)msg.param2), msg.param3, msg.source);
            break;
        case WRITE:
            msg.param1 = filesys_write(msg.param1, va2la(src, (void *)msg.param2), msg.param3, msg.source);
            break;
        case SEEK:
            filesys_seek(msg.param1, msg.param2, msg.param3, msg.source);
            break;
        case TELL:
            msg.param1 = filesys_tell(msg.param1, msg.source);
            break;
        case CREATE:
            msg.param1 = filesys_create((const char *)va2la(src, (void *)msg.param1), msg.param2);
            break;
        case REMOVE:
            msg.param1 = filesys_remove((const char *)va2la(src, (void *)msg.param1));
            break;
        case SYNC:
            cache_sync();
            break;
        default:
            panic("{ FS } unknown msg %d\n", msg.type);
            break;
        }
        send(src, &msg);
    }
}


