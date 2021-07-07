#include "syscall.h"
#include "ipc.h"
#include "../device/console.h"
#include "const.h"


public void halt()
{
    message_t msg;
    msg.type = HALT;
    fsync();
    if(send(TASK_SYS, &msg) == SEND_SUCCESS)
        receive(TASK_SYS, &msg);
}

public void reboot()
{
    message_t msg;
    msg.type = REBOOT;
    fsync();
    if(send(TASK_SYS, &msg) == SEND_SUCCESS)
        receive(TASK_SYS, &msg);
}

public filedesc_t fopen(const char * fname, uint32_t mode)
{
    message_t msg;
    msg.type = OPEN;
    msg.param1 = (int)fname;
    msg.param2 = mode;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
    return msg.param1;
}
public void fclose(filedesc_t fd)
{
    message_t msg;
    msg.type = CLOSE;
    msg.param1 = fd;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}
public size_t fread(filedesc_t fd, const char * buffer, size_t n)
{
    message_t msg;
    msg.type = READ;
    msg.param1 = fd;
    msg.param2 = (int )buffer;
    msg.param3 = n;

    int dest = fd == 0 ? TASK_TTY : TASK_FS;

    if(send(dest, &msg) == SEND_SUCCESS)
        receive(dest, &msg);
    return msg.param1;
}
public size_t fwrite(filedesc_t fd, const char * buffer, size_t n)
{
    message_t msg;
    msg.type = WRITE;
    msg.param1 = fd;
    msg.param2 = (int )buffer;
    msg.param3 = n;

    int dest = fd == 1 ? TASK_TTY : TASK_FS;

    if(send(dest, &msg) == SEND_SUCCESS)
        receive(dest, &msg);
}
public uint32_t ftell(filedesc_t fd)
{
    message_t msg;
    msg.type = TELL;
    msg.param1 = fd;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
    return msg.param1;
}
public void fseek(filedesc_t fd, int off, int mode)
{
    message_t msg;
    msg.type = SEEK;
    msg.param1 = fd;
    msg.param2 = off;
    msg.param3 = mode;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}
public void fsync()
{
    message_t msg;
    msg.type = SYNC;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}
public void mkdir(const char * dirname)
{
    message_t msg;
    msg.type = CREATE;
    msg.param1 = (int)dirname;
    msg.param2 = 0x41ed;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}
public void fcreate(const char * fname)
{
    message_t msg;
    msg.type = CREATE;
    msg.param1 = (int)fname;
    msg.param2 = 0x81b4;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}
public void fremove(const char * fname)
{
    message_t msg;
    msg.type = REMOVE;
    msg.param1 = (int)fname;

    if(send(TASK_FS, &msg) == SEND_SUCCESS)
        receive(TASK_FS, &msg);
}

public pid_t exec(const char * path, const char * argv[])
{
    message_t msg;
    msg.type = EXEC;
    msg.param1 = (int)path;
    msg.param2 = (int)argv;

    if(send(TASK_SYS, &msg) == SEND_SUCCESS)
        receive(TASK_SYS, &msg);
    return msg.param1;
}
public void exit(int status)
{
    message_t msg;
    msg.type = EXIT;
    msg.param1 = status;

    if(send(TASK_SYS, &msg) == SEND_SUCCESS)
        receive(TASK_SYS, &msg);
}
public int wait(pid_t child)
{
    message_t msg;
    msg.type = WAIT;
    msg.param1 = child;

    if(send(TASK_SYS, &msg) == SEND_SUCCESS)
        receive(TASK_SYS, &msg);
    return msg.param1;
}
