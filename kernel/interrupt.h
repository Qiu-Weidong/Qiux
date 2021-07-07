#ifndef QIUX_INTERRUPT_H_
#define QIUX_INTERRUPT_H_
#include "type.h"


typedef struct INTR_FRAME
{
    // 段寄存器         偏移
    uint16_t gs, : 16; // 仅使用低16位 0
    uint16_t fs, : 16; // 4
    uint16_t es, : 16; // 8
    uint16_t ds, : 16; // 12

    // 寄存器
    uint32_t edi;       // 16
    uint32_t esi;       // 20
    uint32_t ebp;       // 24
    uint32_t esp_dummy; // 28 pushad保存的esp，未使用
    uint32_t ebx;       // 32
    uint32_t edx;       // 36
    uint32_t ecx;       // 40
    uint32_t eax;       // 44

    uint32_t retaddr; // 48 call save的时候保存的返回地址

    union
    {
        uint32_t vec_no;
        uint32_t err_code;
    } no_code; // 52 对于有错误码的异常，该字段为错误码，否则为中断号

    // CPU压栈
    uint32_t eip;      // 56
    uint16_t cs, : 16; // 60
    uint32_t eflags;   // 64
    uint32_t esp;      // 68
    uint16_t ss, : 16; // 72

} intr_frame;           // sizeof(intr_frame) = 76

typedef void (*intr_handler)(const intr_frame *, const process_t *);


public void idt_init();

public bool_t is_intr_on();

public bool_t intr_reenter();

public bool_t intr_enable();

public bool_t intr_disable();

public void intr_reset(bool_t intr_flag);

public void idt_register(uint32_t index, intr_handler handler);


#endif // QIUX_INTERRUPT_H_

