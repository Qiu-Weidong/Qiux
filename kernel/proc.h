#ifndef QIUX_PROC_H_
#define QIUX_PROC_H_
#include "type.h"
#include "interrupt.h"
#include "list.h"
#include "ipc.h"
#include "const.h"


typedef unsigned int pid_t;
typedef unsigned int priority_t;

typedef enum PROCESS_STATUS
{
    // NOT_ALLOCATED = 0,              // 待分配
    
    PROC_CREATED  = 1,
    PROC_READY    = 2,              // 就绪中
    PROC_RUNNING  = 3,              // 运行中
    PROC_DEAD     = 4,              // 结束 0000 0100

    // 都是等待状态，将第3位置为1，表示是阻塞状态，阻塞状态可以叠加
    PROC_SENDING     = 0x10,        // 发送中 0001 1000
    PROC_RECEIVING   = 0x20,        // 接收中 0010 1000
    PROC_WAITING     = 0x40,        // 等待中 0100 1000
    PROC_SLEEPING    = 0x80,        // 睡眠中 1000 1000
} process_status;

typedef struct PROCESS
{
    intr_frame registers;   // 寄存器快照
    pid_t pid;              // 进程id 76
    selector_t sel_ldt;     // ldt的选择子 80
    uint64_t ldt[LDT_SIZE]; // ldt; 96
    priority_t priority;    // 优先级 100
    int tty;                // 进程的tty号
    process_status status;  // 进程的状态 108

    list_elem ready_elem;
    list_elem hash_elem;
    list_elem slot_elem;

    message_t * p_msg;      // 指向消息体的指针，注意是线性地址，不需要转换
    pid_t recvfrom;         // receiver要等的人是谁
    bool_t has_intr_msg;    // 是否收到中断消息

    list open_files;
    
    list senders;           // 向该进程发消息的进程列表
    list_elem sender_elem;  // senders的元素

    list children;          // 子进程列表
    list_elem child_elem;   // 子进程列表元素

    pid_t parent;
    pid_t wait_for;         // 正在等待的子进程
    int exit_status;        // 进程的退出码
} process_t; 

public void restart_current_process() NO_RETURN;

public void schedule();

public void msg_send(intr_frame * frame, process_t * sender);

public void msg_receive(intr_frame * frame, process_t * receiver);

public int intr_send(pid_t dest);

public void process_init();

public void unblock(process_t * proc, process_status status);

public void block(process_t * proc, process_status status);

public process_t * pid2proc(pid_t pid);

public pid_t process_execute(const char * file_name, const char * argv[], pid_t parent);

public void process_exit(pid_t self, int status);

public void process_wait(pid_t child, pid_t parent);

public void * va2la(pid_t pid, void * va);

#endif // QIUX_PROC_H_

