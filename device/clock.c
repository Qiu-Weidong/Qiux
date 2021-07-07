#include "clock.h"
#include "pic.h"
#include "__asm__.h"
#include "../kernel/interrupt.h"
#include "../kernel/proc.h"


// 滴答数，每次时钟中断自增
private uint64_t ticks;

/*===========================================================================*
 *				clock_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 时钟初始化
 * 内核执行或者TASK_SYS执行
 *****************************************************************************/
public void clock_init()
{
    outb(TIMER_MODE, RATE_GENERATOR);
    outb(TIMER0, 0xff);
    outb(TIMER0, 0xff);

    ticks = 0;
    idt_register(INT_VECTOR_IRQ0, (intr_handler)clock_handler);
    enable_irq(0);
}

/*===========================================================================*
 *				clock_handler				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 时钟中断处理程序
 * 内核执行
 *****************************************************************************/
public void clock_handler(const intr_frame * frame, const process_t * proc)
{
    ticks++;
    if(!intr_reenter()) schedule();
}
