#ifndef QIUX_GDT_H_
#define QIUX_GDT_H_
#include "type.h"

// 描述符的索引
#define	INDEX_DUMMY			0	
#define	INDEX_FLAT_C		1	
#define	INDEX_FLAT_RW		2	
#define	INDEX_VIDEO			3	
#define INDEX_TSS           4
// 从5开始到127都分配给ldt
#define INDEX_LDT           5

// 选择子
#define	SEL_DUMMY		    0x0		
#define	SEL_FLAT_C		    0x08		
#define	SEL_FLAT_RW	        0x10		
#define	SEL_VIDEO		    (0x18|0x3)
#define SEL_TSS             0x20
#define SEL_LDT             0x28

#define	SEL_KERNEL_CS	SEL_FLAT_C
#define	SEL_KERNEL_DS	SEL_FLAT_RW


public void gdt_init();

public void  gdt_register(uint64_t desc, uint32_t index);

#endif // QIUX_GDT_H_

