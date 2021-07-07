#include "bitmap.h"
#include "super.h"
#include "cache.h"
#include "const.h"
#include "stdio.h"
#include "debug.h"

private uint8_t bitmap_buf[ZONE_SIZE];
extern super_block super;

private int alloc_bit(int drive, int zone_st, int nr_zone);

/*===========================================================================*
 *				alloc_zmap_bit				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 分配扇区
 * 执行进程 TASK_FS
 * 调用函数 inode_write、inode_create
 *****************************************************************************/
public int alloc_zmap_bit(int drive) // 分配zone, 返回分配的zone号
{
    int zone_nr = alloc_bit(drive, super.imap_blk+2, super.zmap_blk);
    if(zone_nr == -1) return zone_nr;
    return zone_nr + super.fst_data_zone;
}

/*===========================================================================*
 *				free_zmap_bit				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 释放扇区
 * 执行进程 TASK_FS
 * 调用函数 inode_trunc、inode_remove
 *****************************************************************************/
public void free_zmap_bit(int drive, int nr_zone)
{
    nr_zone -= super.fst_data_zone;
    nr_zone += 1; // 加一是因为第一个bit未使用
    int nr_byte = nr_zone / 8; // 字节偏移
    int mask = ~(1 << (nr_zone % 8)); // 位偏移
    int bitmap_zone = nr_byte / ZONE_SIZE + 2 + super.imap_blk;
    nr_byte %= ZONE_SIZE;
    uint8_t c;
    cache_read(drive, bitmap_zone, 1, nr_byte, &c);
    c &= mask;
    cache_write(drive, bitmap_zone, 1, nr_byte, &c);
}

/*===========================================================================*
 *				alloc_imap_bit				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 分配一个inode，空间不足返回-1，否则返回分配的inode号
 * 执行进程 TASK_FS
 *****************************************************************************/
public int alloc_imap_bit(int drive)  // 返回ino
{
    int ino = alloc_bit(drive, 2, super.imap_blk);
    if(ino == -1) return ino;
    return ino + 1;
}

/*===========================================================================*
 *				free_imap_bit				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 初始化tty
 * 执行进程 TASK_FS
 * 调用函数 inode_remove
 *****************************************************************************/
public int free_imap_bit(int drive, int ino) // 注意ino从1开始
{
    int nr_byte = ino / 8;
    int mask = ~(1 << (ino % 8));
    int bitmap_zone = nr_byte / ZONE_SIZE + 2;
    nr_byte %= ZONE_SIZE;
    uint8_t c;
    cache_read(drive, bitmap_zone, 1, nr_byte, &c);
    c &= mask;
    cache_write(drive, bitmap_zone, 1, nr_byte, &c);
}

private int alloc_bit(int drive, int zone_st, int nr_zone)
{
    for(int i=0;i<nr_zone;i++) // 遍历zone bitmap
    {
        cache_read(drive, i+zone_st, ZONE_SIZE, 0, bitmap_buf);
        for(int j=0;j<ZONE_SIZE;j++)
        {
            if(bitmap_buf[j] != 0xff) // 如果有可以用的块
            {
                char mask = ~bitmap_buf[j]; // 取反后，可用的位为1
                int result = (i * 1024 + j) * 8;
                int t = 1;
                while((t & mask) == 0)
                {
                    t <<= 1;
                    result++;
                }
                char c =  bitmap_buf[j] | t; // 将这一位置为1
                cache_write(drive, i+zone_st, 1, j, &c); // 写回
                return result-1; // 减去1是因为第一个bit不使用
            }
        }
    }
    return -1;
}
