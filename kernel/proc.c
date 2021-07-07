#include "proc.h"
#include "gdt.h"
#include "const.h"
#include "desc.h"
#include "debug.h"
#include "../include/stdio.h"
#include "syscall.h"
#include "../include/string.h"
#include "__asm__.h"
#include "elf.h"
#include "page.h"

#define MAX_PROC_CNT 128
#define TASK_PRI     0
#define PROC_PRI     31

// 私有数据
private list ready_list; // 就绪队列
private list proc_slot;  // 用于分配空的pcb
private process_t proc_list[MAX_PROC_CNT]; // pcb内存空间
private list proc_hash[MAX_PROC_CNT]; // 用于通过pid获取pcb
private uint8_t stack[6][4096];

// 公有数据
public process_t volatile *current_proc = nullptr;

// 外部函数
public void task_sys();
public void task_ide();
public void task_fs();
public void task_tty();
public void task_shell() NO_OPTIMIZE;
public void task_idle();

/*===========================================================================*
 *				pid2proc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 通过pid获取进程控制块
 * 内核执行
 * 执行进程 TASK_SYS、TASK_FS、TASK_TTY
 *****************************************************************************/
public process_t *pid2proc(pid_t pid)
{
    list * l = &proc_hash[pid % MAX_PROC_CNT];
    for(list_elem * e = list_begin(l); e != list_end(l);
        e = list_next(e))
    {
        process_t * proc = list_entry(e, process_t, hash_elem);
        if(proc->pid == pid) return proc;
    }
    return nullptr;
}

/*===========================================================================*
 *				process_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 初始化proc_list并创建系统任务
 * 内核执行
 *****************************************************************************/
public void process_init()
{
    list_init(&ready_list);
    list_init(&proc_slot);

    idt_register(INT_VECTOR_SEND, (intr_handler)msg_send);
    idt_register(INT_VECTOR_RECEIVE, (intr_handler)msg_receive);
    for (int i = 0; i < MAX_PROC_CNT; i++)
    {
        list_init(&proc_hash[i]);
        list_push_back(&proc_slot, &proc_list[i].slot_elem);
    }

    void * proc_fun[] = { task_sys, task_ide, task_fs, task_tty, task_shell, task_idle };

    for(int i=0;i<=TASK_IDLE;i++)
    {
        process_t *proc = list_entry(list_pop_front(&proc_slot), process_t, slot_elem);
        list_push_back(&proc_hash[i], &proc->hash_elem);

        proc->pid = i;
        proc->priority = TASK_PRI;
        proc->registers.cs = (0 << 3) + SA_RPL1 + SA_TIL;
        proc->registers.ds = proc->registers.es = proc->registers.ss =
            proc->registers.fs = proc->registers.gs = (1 << 3) + SA_RPL1 + SA_TIL;
        proc->registers.eflags = IOPL1_FLAG | INTR_FLAG | MBS_FLAG;
        proc->registers.eip = (uint32_t)proc_fun[i];
        proc->registers.esp = (uint32_t)(stack[i]) + 4096;
        proc->ldt[INDEX_LDT_C] = make_seg_desc(0, 0xfffff, DA_32 | DA_CR | DA_DPL1 | DA_LIMIT_4K);
        proc->ldt[INDEX_LDT_RW] = make_seg_desc(0, 0xfffff, DA_32 | DA_DRW | DA_DPL1 | DA_LIMIT_4K);
        descriptor_t desc = make_ldt_desc((uint32_t)proc->ldt, sizeof(uint64_t) * 2 - 1, DA_32 | DA_DPL0);
        proc->sel_ldt = (INDEX_LDT + i ) << 3;

        gdt_register(desc, INDEX_LDT + i);

        proc->status = PROC_READY;
        proc->p_msg = nullptr;
        proc->has_intr_msg = false;
        proc->tty = 0;
        list_init(&proc->senders);
        list_init(&proc->open_files);
        list_init(&proc->children);
        list_push_back(&ready_list, &proc->ready_elem);
    }
    current_proc = pid2proc(0);
    list_remove((list_elem *)&current_proc->ready_elem);
    current_proc->status = PROC_RUNNING;
}

/*===========================================================================*
 *				process_execute				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 加载程序并执行
 * 执行进程 TASK_SYS
 *****************************************************************************/
public pid_t process_execute(const char *file_name, const char *argv[], pid_t parent)
{
    if(list_empty(&proc_slot)) return -1;

    filedesc_t fd = fopen(file_name, 0);
    Elf32_Ehdr ehdr;
    fread(fd, (const char *)&ehdr, sizeof(Elf32_Ehdr));
    fseek(fd, ehdr.e_phoff, FSEEK_SET);
    Elf32_Phdr phdr;
    int max_addr = 0;
    for (int i = 0; i < ehdr.e_phnum; i++)
    {
        fread(fd, (const char *)&phdr, sizeof(Elf32_Phdr));
        if (phdr.p_type != 1)
            continue;
        max_addr = phdr.p_paddr + phdr.p_memsz > max_addr ? phdr.p_paddr + phdr.p_memsz : max_addr;
    }
    int nr_pages = (max_addr + 4095) / 4096 + 1; // 加1是分配栈空间

    void *addr = palloc(nr_pages);
    if(addr == nullptr) return -1;

    static pid_t next_pid = TASK_IDLE + 1;
    process_t *proc = list_entry(list_pop_front(&proc_slot), process_t, slot_elem); 
    list_push_back(&proc_hash[next_pid % MAX_PROC_CNT], &proc->hash_elem);
    proc->status = PROC_CREATED;
    proc->pid = next_pid++;
    proc->priority = PROC_PRI;
    proc->registers.cs = (0 << 3) + SA_RPL3 + SA_TIL;
    proc->registers.ds = proc->registers.es = proc->registers.ss =
        proc->registers.fs = proc->registers.gs = (1 << 3) + SA_RPL3 + SA_TIL;
    proc->registers.eflags = IOPL3_FLAG | INTR_FLAG | MBS_FLAG;

    proc->sel_ldt = (INDEX_LDT + proc->pid % MAX_PROC_CNT) << 3;
    descriptor_t desc = make_ldt_desc((uint32_t)proc->ldt, sizeof(uint64_t) * 2 - 1, DA_32 | DA_DPL0);
    gdt_register(desc, INDEX_LDT + proc->pid % MAX_PROC_CNT);

    list_init(&proc->senders);
    list_init(&proc->open_files);
    list_init(&proc->children);

    proc->parent = parent;
    process_t * p = pid2proc(parent);
    proc->tty = p->tty;
    list_push_back(&p->children, &proc->child_elem);
    proc->ldt[INDEX_LDT_C] = make_seg_desc((uint32_t)addr, nr_pages, DA_32 | DA_CR | DA_DPL3 | DA_LIMIT_4K); // 用户进程的代码段描述符
    proc->ldt[INDEX_LDT_RW] = make_seg_desc((uint32_t)addr, nr_pages, DA_32 | DA_DRW | DA_DPL3 | DA_LIMIT_4K);

    // 加载文件
    for (int i = 0; i < ehdr.e_phnum; i++)
    {
        fseek(fd, ehdr.e_phentsize * i + ehdr.e_phoff, FSEEK_SET);
        fread(fd, (const char *)&phdr, sizeof(Elf32_Phdr));
        if (phdr.p_type != 1)
            continue;
        memset(addr + phdr.p_paddr, 0, phdr.p_memsz);
        fseek(fd, phdr.p_offset, FSEEK_SET);
        fread(fd, addr + phdr.p_paddr, phdr.p_filesz);
    }
    fclose(fd);

    // 设置eip和esp

    uint32_t esp = nr_pages * 4096;
    int argc = 0;
    // 将参数压入栈中
    while (argv[argc])
    {
        int n = strlen(argv[argc]) + 1;
        esp -= n;
        memcpy(addr + esp, argv[argc], n);
        argv[argc] = (const char *)esp;
        argc++;
    }

    esp -= sizeof(const char *) * argc;
    memcpy(addr + esp, argv, sizeof(const char *) * argc);

    proc->registers.eax = esp;
    proc->registers.ecx = argc;
    proc->registers.eip = ehdr.e_entry;
    proc->registers.esp = esp;

    proc->status = PROC_READY;

    bool_t flag = intr_disable();
    list_push_back(&ready_list, &proc->ready_elem); // 放入就绪进程列表, 所有对read_list的操作都要关中断
    intr_reset(flag);

    return proc->pid;
}

/*===========================================================================*
 *				process_wait				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 等待子进程执行结束并返回子进程的退出码
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void process_wait(pid_t child, pid_t parent)
{
    message_t msg;
    process_t *p = pid2proc(parent);
    // 如果正在等待或者父进程不存在子进程，那么直接返回
    if ((p->status & PROC_WAITING) || list_empty(&p->children))
    {
        msg.param1 = -1;
        send(parent, &msg);
        return;
    }

    process_t *c = nullptr;

    if (child == ANY)
    {
        // 如果等待任一子进程，那么先找有没有已经结束的
        for (list_elem * e = list_begin(&p->children); e != list_end(&p->children);
             e = list_next(e))
        {
            process_t *proc = list_entry(e, process_t, child_elem);
            if (proc->status == PROC_DEAD)
            {
                c = proc;
                break;
            }
        }

    }
    else // 否则查找要等待的子进程
    {
        for (list_elem * e = list_begin(&p->children); e != list_end(&p->children);
             e = list_next(e))
        {
            process_t *proc = list_entry(e, process_t, child_elem);
            if (proc->pid == child)
            {
                c = proc;
                break;
            }
        }
        if (c == nullptr)
        {
            msg.param1 = -1;
            send(parent, &msg);
            return ;
        }
    }

    
    
    /*
     * 此时有3种可能：
     * 1、 child == ANY，c == nullptr , 暂时还没有已经结束的子进程
     * 2、 child == ANY，c != nullptr , 有已经结束的子进程
     * 3、 child != ANY, c != nullptr , 要等待的进程还没有结束
    */
    if (c == nullptr || c->status != PROC_DEAD)
    {// 阻塞父进程
        bool_t flag = intr_disable();

        p->wait_for = child;
        assert(p->status != PROC_RUNNING);
        if(p->status == PROC_READY) list_remove(&p->ready_elem);
        p->status |= PROC_WAITING;
        intr_reset(flag);
    }
    else {
        list_remove(&c->hash_elem);
        list_push_back(&proc_slot, &c->slot_elem);
        list_remove(&c->child_elem);
        msg.param1 = c->exit_status;
        send(parent, &msg);
    }
    
}

/*===========================================================================*
 *				process_exit				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 以status状态推出
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void process_exit(pid_t self, int status)
{
    process_t *proc = pid2proc(self);
    if (proc->status == PROC_READY)
    {
        bool_t flag = intr_disable();
        list_remove(&proc->ready_elem);
        intr_reset(flag);
    }
    proc->status = PROC_DEAD;

    void *base_mem = get_base_from_desc(proc->ldt[0]);
    uint32_t limit = get_limit_from_desc(proc->ldt[0]);
    pfree(base_mem, limit / 4096);

    proc->exit_status = status;

    for(list_elem * e = list_begin(&proc->children); e != list_end(&proc->children);
        e = list_next(e))
    { // 将子进程都设置为孤儿进程
        process_t * p = list_entry(e, process_t, child_elem);
        p->parent = NO_PROC;
    }

    process_t * parent = pid2proc(proc->parent);
    if(proc->parent == NO_PROC || parent  == nullptr)
    {
        list_remove(&proc->hash_elem);
        list_push_back(&proc_slot, &proc->slot_elem);
        return;
    }
    
    if((parent->status & PROC_WAITING) && 
        (parent->wait_for == ANY || parent->wait_for == self))
    {
        parent->wait_for = self;
        
        bool_t flag = intr_disable();
        parent->status &= ~PROC_WAITING;
        if(!(parent->status & 0xf0))            // 如果不处于阻塞状态
        {
            parent->status = PROC_READY;
            list_push_back(&ready_list, &parent->ready_elem);
        }
        intr_reset(flag);
        
        list_remove(&proc->hash_elem);
        list_push_back(&proc_slot, &proc->slot_elem);
        list_remove(&proc->child_elem);
        message_t msg;
        msg.param1 = status;
        send(proc->parent, &msg);
        
    }

}

/*===========================================================================*
 *				va2la				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 将逻辑地址转换为线性地址
 * 执行进程 TASK_SYS、TASK_TTY、TASK_IDE、TASK_FS
 *****************************************************************************/
public void *va2la(pid_t pid, void *va)
{
    process_t *proc = pid2proc(pid);
    void *seg_base = get_base_from_desc(proc->ldt[INDEX_LDT_RW]);
    return (void *)((uint32_t)seg_base + (uint32_t)va);
}

/*===========================================================================*
 *				schedule				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 进程调度
 * 执行进程 TASK_SYS、TASK_TTY、TASK_IDE、TASK_FS
 *****************************************************************************/
public void schedule()
{
    // 从就绪队列中选择优先级最高的一个运行，并将当前进程放入就绪队列
    if (current_proc->status == PROC_RUNNING)
    {
        current_proc->status = PROC_READY;
        list_push_back(&ready_list, (list_elem *)&current_proc->ready_elem);
    }
    if (list_empty(&ready_list))
        panic("No Ready Process!!!");
    list_elem *e = list_pop_front(&ready_list);
    process_t * proc = list_entry(e, process_t, ready_elem);
    assert(proc->status == PROC_READY);
    current_proc = proc;
    current_proc->status = PROC_RUNNING;
}

/*===========================================================================*
 *				msg_send				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 消息发送
 * 内核执行
 *****************************************************************************/
public void msg_send(intr_frame *frame, process_t *sender)
{
    pid_t dest = (pid_t)frame->eax;
    message_t *msg = (message_t *)frame->ecx;

    process_t *receiver = pid2proc(dest);
    if (receiver == nullptr) // 您要找的人不存在
    {
        frame->eax = -1;
        return;
    }
    msg = (message_t *)va2la(sender->pid, (void *)msg);
    msg->source = sender->pid;  // 相当于在信上署名
    sender->p_msg = msg;        // 将信揣好, 注意进程pcb中的p_msg属性是转换后的地址
    assert(receiver != sender); // 不能给自己发消息

    // 当sender走到receiver的门口时
    // receiver已经在门口等待了，并且等的人就是sender(或者等待任何人)
    if ((receiver->status & PROC_RECEIVING) &&
        (receiver->recvfrom == ANY || receiver->recvfrom == sender->pid))
    {
        assert(receiver->p_msg);                                   // 确保receiver有东西装
        *(receiver->p_msg) = *(sender->p_msg);
        unblock(receiver, PROC_RECEIVING);                                         // 告诉receiver可以回去了
    }
    // 如果receiver不在家，则在家门口等待receiver。
    // 可能有其他的送信人已经在等待了，所以需要排队
    else
    {
        // 将sender添加到receiver的sender列表中
        block(sender, PROC_SENDING);
        list_push_back(&receiver->senders, &sender->sender_elem);
    }
    frame->eax = 0;
}

/*===========================================================================*
 *				msg_receive				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 消息接收
 * 内核执行
 *****************************************************************************/
public void msg_receive(intr_frame *frame, process_t *receiver)
{
    pid_t src = (pid_t)frame->eax;
    message_t *msg = (message_t *)frame->ecx;

    msg = (message_t *)va2la(receiver->pid, (void *)msg);
    assert(receiver->pid != src);
    receiver->p_msg = msg;
    if (src != ANY && src != INTERRUPT && pid2proc(src) == nullptr) // 您要找的人不存在
    {
        frame->eax = -1;
        return;
    }
    if (receiver->has_intr_msg && (src == ANY || src == INTERRUPT))
    {
        receiver->has_intr_msg = false;
        msg->source = INTERRUPT;
        msg->type = HARD_INT;
        frame->eax = 0;
        return;
    }
    process_t *sender = nullptr;
    // receiver一出门，就看到有送信人在排队
    if (!list_empty(&receiver->senders) && src != INTERRUPT)
    {
        if (src == ANY) // 如果receiver打算接受任何人的信，那么就先接收第一个来的人的。
        {
            sender = list_entry(list_pop_front(&receiver->senders), process_t, sender_elem);
        }
        else
        { // 否则在送信人当中寻找自己要等的人
            for (list_elem *e = list_begin(&receiver->senders);
                 e != list_end(&receiver->senders); e = list_next(e))
            {
                process_t *p = list_entry(e, process_t, sender_elem);
                assert(p->status & PROC_SENDING);
                if (p->pid == src)
                {
                    sender = p;
                    list_remove(&sender->sender_elem);
                    break;
                }
            }
        }
    }

    // receiver要等的人还没来, 或者要接收中断信息
    if (sender == nullptr)
    { // 如果这时来了中断，并将has_intr_msg置为1
        bool_t intr_level = intr_disable();

        if (receiver->has_intr_msg && (src == ANY || src == INTERRUPT))
        {
            receiver->has_intr_msg = false;
            msg->source = INTERRUPT;
            msg->type = HARD_INT;
            frame->eax = 0;
            return;
        }
        else
        {
            receiver->recvfrom = src;
            block(receiver, PROC_RECEIVING); // 接着等
        }
        intr_reset(intr_level);
    }
    // receiver要等的人来了
    else
    {
        assert(sender->status & PROC_SENDING);
        *(receiver->p_msg) = *(sender->p_msg);
        unblock(sender, PROC_SENDING);                                           // 告诉送信人可以走了
    }

    frame->eax = 0;
    return;
}

/*===========================================================================*
 *				intr_send				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 中断消息发送
 * 内核执行
 *****************************************************************************/
public int intr_send(pid_t dest)
{
    process_t *receiver = pid2proc(dest);
    if (receiver == nullptr)
        return -1;

    else if ((receiver->status & PROC_RECEIVING) &&
             (receiver->recvfrom == INTERRUPT || receiver->recvfrom == ANY))
    {
        receiver->has_intr_msg = false;
        receiver->p_msg->source = INTERRUPT;
        receiver->p_msg->type = HARD_INT;
        unblock(receiver, PROC_RECEIVING);
    }
    else
    {
        receiver->has_intr_msg = true;
    }
}

/*===========================================================================*
 *				unblock				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 解除阻塞
 * 内核执行
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void unblock(process_t *proc, process_status status)
{
    proc->status &= ~status;
    if(!(proc->status & 0xf0))
    {
        proc->status = PROC_READY;
        list_push_back(&ready_list, &proc->ready_elem); // 如果这时发生时钟中断，可能出错
    }
}

/*===========================================================================*
 *				block				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 阻塞进程
 * 内核执行
 * 执行进程 TASK_SYS
 *****************************************************************************/
public void block(process_t *proc, process_status status)
{
    if (proc->status == PROC_READY)
    {
        list_remove(&proc->ready_elem);
    }
    else if(proc->status == PROC_RUNNING)
    {
        proc->status |= status;
        schedule();
    }
    else proc->status |= status;
    
}
