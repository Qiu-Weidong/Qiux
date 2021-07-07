#ifndef QIUX_IPC_H_
#define QIUX_IPC_H_
#include "interrupt.h"

// 定义一些ipc返回结果常量
#define SEND_SUCCESS 0
#define RECEIVE_SUCCESS 0
#define SEND_ERROR -1
#define RECEIVE_ERROR -1


typedef enum MSGTYPE
{
    HARD_INT = 1, // 硬件中断
	
	/* 系统调用 */
    HALT,
    REBOOT,

	EXEC,
	EXIT,
	WAIT,

	/* 文件系统 */
	OPEN,
	CLOSE,
	READ,
	WRITE,
	SEEK,
	TELL,
	SYNC,
	CREATE,
	REMOVE,

	/* message type for drivers */
	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL,
	DEV_SELECT,
	DEV_CANCEL
} msgtype;

typedef struct MESSAGE
{
    pid_t source; 	// sender的pid
    msgtype type;   // 消息类型，如系统调用号

	int param1;		/* \           */  // param1同时用于返回
	int param2;		/*  | 用于传参  */
	int param3;		/*  |          */
	int param4;		/* /           */

} message_t;

public int send(pid_t dest, message_t * msg);
public int receive(pid_t src, message_t * msg);
static inline int sendrecv(pid_t src_dest, message_t * msg)
{
	if(send(src_dest, msg) == SEND_SUCCESS) return receive(src_dest, msg);
	return SEND_ERROR;
}
#endif // QIUX_IPC_H_

