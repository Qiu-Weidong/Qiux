#include "interrupt.h"
#include "__asm__.h"
#include "const.h"
#include "desc.h"
#include "stdlib.h"
#include "../include/stdio.h"
#include "../device/pic.h"

#define IDT_SIZE 256
// 公有变量
public intr_handler intr_handlers[IDT_SIZE];
public int32_t volatile k_reenter;

// 外部变量
extern void (* intr_stubs[IDT_SIZE])();

// 私有变量
private descriptor_t idt[IDT_SIZE];
private const char *err_msg[] = {
    "#DE Divide Error",
    "#DB RESERVED",
    "--  NMI Interrupt",
    "#BP Breakpoint",
    "#OF Overflow",
    "#BR BOUND Range Exceeded",
    "#UD Invalid Opcode (Undefined Opcode)",
    "#NM Device Not Available (No Math Coprocessor)",
    "#DF Double Fault",
    "    Coprocessor Segment Overrun (reserved)",
    "#TS Invalid TSS",
    "#NP Segment Not Present",
    "#SS Stack-Segment Fault",
    "#GP General Protection",
    "#PF Page Fault",
    "--  (Intel reserved. Do not use.)",
    "#MF x87 FPU Floating-Point Error (Math Fault)",
    "#AC Alignment Check",
    "#MC Machine Check",
    "#XF SIMD Floating-Point Exception"
};

private void exception_handler(const intr_frame *frame, process_t *); 

/*===========================================================================*
 *				idt_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 初始化idt
 * 内核执行
 *****************************************************************************/
public void idt_init()
{
    // 先初始化8259A
    pic_init();
    k_reenter = 0;
    const selector_t cs_selector = (1 << 3) + SA_RPL0 + SA_TIG;

    // 全部设置为中断门
    for (int i = 0; i < IDT_SIZE; i++)
        idt[i] = make_intr_gate(intr_stubs[i], cs_selector, 0);
    idt[INT_VECTOR_SEND] = make_intr_gate(intr_stubs[INT_VECTOR_SEND], cs_selector, 3);
    idt[INT_VECTOR_RECEIVE] = make_intr_gate(intr_stubs[INT_VECTOR_RECEIVE], cs_selector, 3);

    for (int i = 0; i < IDT_SIZE; i++)
        intr_handlers[i] = (intr_handler)exception_handler;

    uint16_t idt_ptr[3];
    *((uint16_t volatile *)idt_ptr) = sizeof(uint64_t)*IDT_SIZE - 1;
    *((uint32_t volatile *)(idt_ptr + 1)) = (uint32_t)idt;

    lidt(idt_ptr);
}

/*===========================================================================*
 *				idt_register				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 注册中断处理程序
 * 内核执行
 *****************************************************************************/
public void idt_register(uint32_t index, intr_handler handler)
{
    intr_handlers[index] = handler;
}

/*===========================================================================*
 *				intr_reenter				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 判断是否中断重入
 * 内核执行
 *****************************************************************************/
public bool_t intr_reenter()
{
    return k_reenter > 0;
}

/*===========================================================================*
 *				intr_reenter				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 判断是否开启中断
 * 多个进程执行
 *****************************************************************************/
public bool_t is_intr_on()
{
    uint32_t flags = get_eflags();
    return flags & INTR_FLAG;
}

/*===========================================================================*
 *				intr_enable				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 关中断，并返回关闭前中断是否打开
 * 内核执行
 *****************************************************************************/
public bool_t intr_enable()
{
    bool_t result = is_intr_on();
    sti();
    return result;
}

/*===========================================================================*
 *				intr_disable				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 关中断，并返回关闭前中断是否打开
 * 内核执行
 *****************************************************************************/
public bool_t intr_disable()
{
    bool_t result = is_intr_on();
    cli();
    return result;
}

/*===========================================================================*
 *				intr_reset				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> <特权级 1> 中断重置
 * 内核执行
 *****************************************************************************/
public void intr_reset(bool_t intr_flag)
{
    intr_flag ? sti() : cli();
}

/*===========================================================================*
 *				exception_handler				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 一个默认的异常处理程序
 * 内核执行
 *****************************************************************************/
private void exception_handler(const intr_frame *frame, process_t * p_proc)
{
    if(frame->no_code.vec_no >= 0 && frame->no_code.vec_no <= 19)
        printk("%s\n",err_msg[frame->no_code.vec_no]);
    cli();
    hlt();
}
