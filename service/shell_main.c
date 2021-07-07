#include "type.h"
#include "../include/stdio.h"
#include "../kernel/syscall.h"
#include "const.h"
#include "../include/string.h"



public void task_shell() NO_OPTIMIZE;

/*===========================================================================*
 *				task_shell				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1>
 * 所属进程: TASK_SHELL
 *****************************************************************************/
public void task_shell()
{
    int exit_status;

    while(1)
    {
        puts("~/$ ");
        char cmd[128];
        gets(cmd);
        if(cmd[0] == '\0') continue;

        const char * argv[16];
        char * p = cmd;
        int argc = 0;

        while(*p != '\0')
        {
            argv[argc++] = p;
            while(*p != '\0' && *p != ' ') p++;
            if(*p == '\0') break;

            *p++ = '\0';
            while(*p != '0' && *p == ' ') p++;
        }
        argv[argc] = nullptr;
        if(strcmp(argv[0], "$?") == 0) {
            printf("%d\n", exit_status);
            continue;
        }

        pid_t child = exec(argv[0], argv);
        exit_status = wait(child);

    }
}

