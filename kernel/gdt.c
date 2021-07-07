#include "gdt.h"
#include "__asm__.h"
#include "const.h"
#include "tss.h"
#include "desc.h"

#define GDT_SIZE 128

private descriptor_t gdt[GDT_SIZE];

/*===========================================================================*
 *				gdt_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 初始化gdt
 * 内核执行
 *****************************************************************************/
public void gdt_init()
{
    gdt[INDEX_DUMMY] = make_desc(0, 0, 0);
    gdt[INDEX_FLAT_C] = make_seg_desc(0, 0xfffff, DA_32 | DA_CR | DA_DPL0 | DA_LIMIT_4K);
    gdt[INDEX_FLAT_RW] = make_seg_desc(0, 0xfffff, DA_32 | DA_DPL0 | DA_DRW | DA_LIMIT_4K);
    gdt[INDEX_VIDEO] = make_seg_desc(0xb8000, 0xffff, DA_DRW | DA_DPL3);
    gdt[INDEX_TSS] = make_tss_desc((uint32_t)tss_get(), sizeof(tss_t) - 1, 0);

    uint16_t gdt_ptr[3];
    *((uint16_t volatile *)gdt_ptr) = GDT_SIZE * 8 - 1;
    *((uint32_t volatile *)(gdt_ptr + 1)) = (uint32_t)gdt;
    lgdt(gdt_ptr);

    // 更新除cs外的段寄存器，由于我们新定义的gdt与loader中定义的一样，省略掉似乎也没事
    set_ds(SEL_KERNEL_DS);
    set_es(SEL_KERNEL_DS);
    set_fs(SEL_KERNEL_DS);
    set_ss(SEL_KERNEL_DS);
    set_gs(SEL_VIDEO);
}

/*===========================================================================*
 *				gdt_register				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 注册gdt描述符
 * 内核执行
 *****************************************************************************/
public void gdt_register(uint64_t desc, uint32_t index)
{
    gdt[index] = desc;
}
