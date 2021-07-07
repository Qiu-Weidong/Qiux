#include "page.h"
#include "../include/string.h"
#include "elf.h"

typedef struct 
{
    uint32_t BaseAddrLow;
    uint32_t BaseAddrHigh;
    uint32_t LengthLow;
    uint32_t LengthHigh;
    uint32_t Type;
} ARDS;

private void * page_bitmap;
private int page_bitmap_size;


private void bitmap_set(int page_idx);
private void bitmap_clear(int page_idx);
private void bitmap_set_multiple(int start_idx, int end_idx); // 左闭右开
private void bitmap_clear_multiple(int start_idx, int end_idx); // 左闭右开

/*===========================================================================*
 *				page_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 0> 初始化页分配功能
 * 内核执行
 *****************************************************************************/
public void page_init()
{
    uint32_t dwMCRNumber = *(uint32_t *)(0x88fc);
    ARDS * ards = (ARDS *)0x8900;

    // 计算可用内存的总大小和最大地址
    int mem_total = 0;
    int max_addr = 0;
    for(int i=0;i<dwMCRNumber;i++)
    {
        if(ards[i].Type != 1) continue; // Type == 1的内存才是我们可以使用的
        
        mem_total += ards[i].LengthLow;
        if(max_addr < ards[i].BaseAddrLow + ards[i].LengthLow)
            max_addr = ards[i].BaseAddrLow + ards[i].LengthLow;
    }

    int total_pages = max_addr / 4096;
    int bitmap_pages = (total_pages + 8*4096-1) / (8*4096);
    page_bitmap_size = bitmap_pages * 4096;
    page_bitmap = (void *)(max_addr - page_bitmap_size); // 将page_bitmap放在最高地址处
    // page_bitmap : 0x1fef000 1ff0000 (e8 1D)
    // 这些地址共分为max_addr / 4096页，需要bitmap大小为 max_addr / (4096 * 8) 字节
    // 则bitmap占用的页数为max_addr / (4096 * 8 * 4096)

    // 一页bitmap可以映射8*4096页内存

    // 先将bitmap清0
    memset(page_bitmap, 0, page_bitmap_size);

    // 将超出max_addr的页全部置为1
    int start_idx = (int)page_bitmap / 4096 ; // page_idx;
    int end_idx = page_bitmap_size * 8;
    bitmap_set_multiple(start_idx, end_idx);

    // 将Type != 1 的内存段置为1
    for(int i=0;i<dwMCRNumber;i++)
    {
        if(ards[i].Type == 1) continue; // Type == 1的内存才是我们可以使用的
        
        start_idx = ards[i].BaseAddrLow  / 4096;
        end_idx = (ards[i].BaseAddrLow + ards[i].LengthLow + 4095) / 4096;
        bitmap_set_multiple(start_idx, end_idx); 
    }

    // 将内核使用过的内存置为1
    Elf32_Ehdr * kernel_header = (Elf32_Ehdr *)0x70000;
    Elf32_Phdr * pht = (Elf32_Phdr *)(0x70000 + kernel_header->e_phoff);
    for(int i=0;i<kernel_header->e_phnum;i++) // 遍历program header table
    {
        if(pht[i].p_type == 0x1) // type为load
        {
            start_idx = pht[i].p_paddr / 4096;
            end_idx = (pht[i].p_paddr + pht[i].p_memsz + 4095) / 4096;
            bitmap_set_multiple(start_idx, end_idx);
        }
    }

    // 将1M以下的地址置为1
    bitmap_set_multiple(0, 1024*1024 / 4096);
}

private inline int left_bit(uint8_t c)
{
    // 从高地址开始计算，最多的空位
    int result = 0;
    uint8_t mask = 0x80;
    while(!(mask & c) && result < 8)
    {
        result++;
        mask >>= 1;
    }
    return result;
}
private inline int right_bit(uint8_t c)
{
    // 从低地址开始计算，最多的空位
    int result = 0;
    uint8_t mask = 0x01;

    while(!(mask & c) && result < 8)
    {
        mask <<= 1;
        result++;
    }
    return result;
}
private inline int bit(uint8_t c, int n) // 在一个字节中分配n位，返回位下标，分配不了返回-1
{
    // 从任意位置计算，最多的空位
    int result = 0;
    // n >= 1 && n <= 8
    uint8_t masks[] = {0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };
    uint8_t mask = masks[n-1];
    while(result <= 8 - n)
    {
        if(mask == (mask & ~c)) return result;
        mask <<= 1;
        result++;
    }
    return -1;
}

/*===========================================================================*
 *				palloc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 按页分配内存
 * 执行进程 TASK_SYS
 * 调用函数 process_execute
 *****************************************************************************/
public void * palloc(int nr_pages)
{
    // while(test_and_set(&lock)) ;
    uint8_t * st = page_bitmap;

    int t = (nr_pages+7) / 8; // 至少需要t字节的bitmap，最多需要t+1字节
    
    int ok = 0;
    // t == 1 需要特判
    if(t == 1)
    {
        for(st=page_bitmap;st<(uint8_t *)page_bitmap+page_bitmap_size;st++)
        {
            if(bit(*st, nr_pages) != -1)
            {
                // 一个字节就够分配了
                ok = 1;
                break;
            }
            else if(st+1<(uint8_t *)page_bitmap+page_bitmap_size 
                && left_bit(*st) + right_bit(*(st+1)) >= nr_pages)
            {
                // 分配两个字节
                ok = 1;
                t++;
                break;
            }
        }
    }
    
    if(ok)
    {
        int k = t==1 ? bit(*st, nr_pages) : 8 - left_bit(*st);
        int page_idx = 8*(st-(uint8_t *)page_bitmap) + k;
        bitmap_set_multiple(page_idx, page_idx+nr_pages);
        // atomic_clear(&lock);
        return (void *)(page_idx * 4096);
    }
    // t >= 2 时, *st取left，*(st+t-1)取right，中间*(st+1)到*(st+t-2)取0
    for(st=page_bitmap;st+t-1<(uint8_t *)page_bitmap+page_bitmap_size;st++)
    {
        int flag = 0;
        for(uint8_t * p=st+1;p<st+t-1;p++) if(*p != 0) {flag = 1; break;}
        if(flag) continue;
        if(left_bit(*st)+right_bit(*(st+t-1)) + 8*(t-2) >= nr_pages)
        {
            ok = 1;
            break;
        }
        // *st取left，*(st+t)取right
        else if(st+t<(uint8_t *)page_bitmap+page_bitmap_size && *(st+t-1)==0 && 
            left_bit(*st)+right_bit(*(st+t)) + 8*(t-1) >= nr_pages)
        {
            ok = 1;
            t++;
            break;
        }
    }

    if(ok)
    {
        int k = left_bit(*st);
        int page_idx = 8*(st-(uint8_t *)page_bitmap) + 8 - k;
        bitmap_set_multiple(page_idx, page_idx+nr_pages);
        // atomic_clear(&lock);
        return (void *)(page_idx * 4096);
    }
    // atomic_clear(&lock);
    return nullptr;
}

/*===========================================================================*
 *				pfree				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 按页释放内存
 * 执行进程 TASK_SYS
 * 调用函数 process_exit
 *****************************************************************************/
public void pfree(void * addr, int nr_pages)
{
    int start_idx = (int) addr / 4096;
    bitmap_clear_multiple(start_idx, start_idx + nr_pages);
}

private void bitmap_set_multiple(int start_idx, int end_idx)
{
    int bytes_st = start_idx / 8;
    int bytes_ed = end_idx / 8;
    uint8_t mask_st = 0xff << (start_idx % 8);
    uint8_t mask_ed = ~(0xff << (end_idx % 8));

    uint8_t * p = page_bitmap + bytes_st;
    if(bytes_st == bytes_ed)
        *p |= (mask_ed & mask_st);
    else {
        *p |= mask_st;
        for(p++;p<(uint8_t *)page_bitmap+bytes_ed;p++)
            *p = 0xff;
        *p |= mask_ed;
    }
}

private void bitmap_clear_multiple(int start_idx, int end_idx)
{
    int bytes_st = start_idx / 8;
    int bytes_ed = end_idx / 8;
    uint8_t mask_st = ~(0xff << (start_idx % 8));
    uint8_t mask_ed = (0xff << (end_idx % 8));

    uint8_t * p = page_bitmap + bytes_st;
    if(bytes_st == bytes_ed)
        *p &= (mask_ed | mask_st);
    else {
        *p &= mask_st;
        for(p++;p<(uint8_t *)page_bitmap+bytes_ed;p++)
            *p = 0x00;
        *p &= mask_ed;
    }
}
