#include "type.h"
#include "__asm__.h"
#include "gdt.h"
#include "tss.h"
#include "interrupt.h"
#include "proc.h"
#include "page.h"
#include "../device/pic.h"
#include "debug.h"



/*===========================================================================*
 *				kernel_main				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 内核主函数
 * 内核执行
 *****************************************************************************/
public int kernel_main()
{
    gdt_init();
    idt_init();
    page_init();
    process_init();
    tss_init();
    
    restart_current_process(); // 启动进程
    hlt();
}
