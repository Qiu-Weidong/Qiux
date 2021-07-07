#include "inode.h"
#include "const.h"
#include "../kernel/ipc.h"
#include "debug.h"
#include "string.h"
#include "cache.h"
#include "super.h"
#include "bitmap.h"
#include "directory.h"

#define INODE_LIST_SIZE 128
extern super_block super;

private inode_t inode_list[INODE_LIST_SIZE];
private list inode_hash[INODE_LIST_SIZE];
private list inode_slots;

/*===========================================================================*
 *				ino2inode				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 通过ino查找inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public inode_t * ino2inode(int drive, int ino)
{
    list * l = &inode_hash[(ino-1)%INODE_LIST_SIZE];
    for(list_elem * e = list_begin(l); e != list_end(l);
        e = list_next(e))
    {
        inode_t * inode = list_entry(e, inode_t, slot_hash_elem);
        if(inode->dev == drive && inode->ino == ino) return inode;
    }
    return nullptr;
}

/*===========================================================================*
 *				inode_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 初始化inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public void inode_init()
{
    list_init(&inode_slots);

    for(int i=0;i<INODE_LIST_SIZE;i++) 
    {
        inode_list[i].ino = 0;
        inode_list[i].open_cnt = 0;
        list_init(&inode_hash[i]);
        list_push_back(&inode_slots, &inode_list[i].slot_hash_elem);
    }
}

/*===========================================================================*
 *				inode_open				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 打开inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public inode_t *inode_open(int drive, int ino)  // 注意ino从1开始
{
    inode_t * open_inode = ino2inode(drive, ino);
    if(open_inode == nullptr)
    {
        if(list_empty(&inode_slots)) return nullptr;
        open_inode = list_entry(list_pop_front(&inode_slots), inode_t, slot_hash_elem);
        list_push_back(&inode_hash[(ino-1)%INODE_LIST_SIZE], &open_inode->slot_hash_elem);
    }
    else {
        open_inode->open_cnt++;
        return open_inode;
    }
    assert(open_inode != nullptr);

    int inode_per_zone = ZONE_SIZE / sizeof(inode_disk);      // 每个zone中有多少inode                                               
    int zone_nr = 2 + super.imap_blk + super.zmap_blk + (ino - 1) / inode_per_zone; // ino所在的zone号

    int bias = ((ino - 1) % inode_per_zone) * sizeof(inode_disk);// inode在这个zone中的偏移
    // 将其缓存到inode缓存里面
    cache_read(drive, zone_nr, sizeof(inode_disk),bias, open_inode);
    open_inode->dev = drive;
    open_inode->ino = ino;
    open_inode->open_cnt = 1;
    return open_inode;
}

/*===========================================================================*
 *				inode_close				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关闭inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public void inode_close(inode_t *inode)
{
    if (--inode->open_cnt == 0)
    {
        inode->ino = 0;
        list_remove(&inode->slot_hash_elem); // 从hash表中删除
        list_push_back(&inode_slots, &inode->slot_hash_elem); // 插入slots列表中
        cache_sync(); // 写回内存, 也可以最后一起写会磁盘
    }
}

/*===========================================================================*
 *				inode_read				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> inode读数据
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t inode_read(inode_t *inode, void *buffer, size_t size, offset_t offset)
{
    size_t result = 0;
    while (size > 0)     // 还有数据要读
    {
        if (offset >= inode->size)
            return result;                // 如果超过了文件大小，直接返回result
        int nr_zone = offset / ZONE_SIZE; // 在第几块zone当中
        int nr_data = 0;
        if (nr_zone < 7)
        {
            // 前7块是直接块
            nr_data = inode->zone[nr_zone];
        }
        else
        {
            cache_read(inode->dev, inode->zone[7], 2, 2*(nr_zone - 7), &nr_data);
        }

        int k = offset % ZONE_SIZE; // offset是相对于整个文件的偏移，而k是针对zone的偏移
        int t = ZONE_SIZE - k > size ? size : ZONE_SIZE - k;
        cache_read(inode->dev, nr_data, t, k, buffer);
        size -= t;
        offset += t;
        result += t;
        buffer += t;
    }
    return result;
}

/*===========================================================================*
 *				inode_write				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> inode写数据
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t inode_write(inode_t *inode, void * buffer, size_t size, offset_t offset)
{
    size_t result = 0;
    int max_size = inode_get_allocated_size(inode); // 已经分配了的空间
    if(offset + size > max_size) // 如果需要更多的空间
    {
        int cur_zones = max_size / ZONE_SIZE; // 当前这个inode使用了多少zone
        int total_zones = (offset + size + ZONE_SIZE - 1) / ZONE_SIZE; // 写完后需要的zone
        // 是否需要分配一个间接块，如果还没有间接块并且后面需要间接块，就分配
        if(cur_zones <= 7 && total_zones > 7) inode->zone[7] = alloc_imap_bit(inode->dev);
        for(int i=cur_zones; i<total_zones;i++)
        {
            if(i < 7) // 直接块
            {
                inode->zone[i] = alloc_zmap_bit(inode->dev); // 直接分配即可
            }
            else {
                // 间接块
                int zone = alloc_zmap_bit(inode->dev);
                cache_write(inode->dev, inode->zone[7], 2, (i-7)*2, &zone);
            }
        }
    }
    if(inode->size < offset + size)
    {
        inode->size = offset + size;
        inode_sync(inode);
    }

    while(size > 0)
    {// 空间一定是够的
        int nr_zone = offset / ZONE_SIZE;
        int nr_data = 0;
        if(nr_zone < 7)
            nr_data = inode->zone[nr_zone];
        else 
            cache_read(inode->dev, inode->zone[7], 2, 2*(nr_zone-7), &nr_data);
        
        int k = offset % ZONE_SIZE;
        int t = ZONE_SIZE -k > size ? size : ZONE_SIZE - k;
        cache_write(inode->dev, nr_data, t, k, buffer);
        size -= t;
        offset += t;
        result += t;
        buffer += t;
    }
    return result;
}

/*===========================================================================*
 *				inode_trunc				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 截断inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t inode_trunc(inode_t *inode, offset_t offset)
{
    size_t result = 0;
    int max_size = inode_get_allocated_size(inode);
    if(offset >= max_size) return result;
    int nr_zone = (offset + ZONE_SIZE - 1) / ZONE_SIZE; // offset在第几个zone，减一是因为不包括offset 10
    int total_zones = max_size / ZONE_SIZE; // 总共的zone数
    // nr_zone 是第一个被释放的zone号
    int st = nr_zone > 7 ? nr_zone : 7;
    for(int i=st;i<total_zones;i++) // 先释放间接块
    {
        uint16_t nr_data;
        cache_read(inode->dev, inode->zone[7], 2, 2*(i-7), &nr_data);
        free_zmap_bit(inode->dev, nr_data);
        nr_data = 0;
        cache_write(inode->dev, inode->zone[7], 2, 2*(i-7), &nr_data);
    }
    for(int i=nr_zone;i<total_zones && i<=7;i++) // 释放直接块
    {
        free_zmap_bit(inode->dev, inode->zone[i]);
        inode->zone[i] = 0;
    }
    inode->size = offset;
    inode_sync(inode);
}

/*===========================================================================*
 *				inode_create				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 创建inode，返回创建的ino, 创建失败返回-1
 * 执行进程 TASK_FS
 *****************************************************************************/
public int inode_create(int drive, uint16_t mode, uint16_t nr_zone) // 预分配nr_zone个zone
{
    int ino = alloc_imap_bit(drive); // 分配一个inode
    if(ino <= 0) return -1;
    int inodes_per_zone = ZONE_SIZE / sizeof(inode_disk);
    int zone_for_ino = 2+super.imap_blk+super.zmap_blk + (ino - 1) / inodes_per_zone;
    int bias = ((ino - 1) % inodes_per_zone) * sizeof(inode_disk);
    inode_disk new_inode;
    memset(&new_inode, 0, sizeof(inode_disk));

    new_inode.mode = mode;
    new_inode.nlinks = 1;
    for(int i=0;i<nr_zone && i<7;i++)
    {
        new_inode.zone[i] = alloc_zmap_bit(drive);
    }
    if(nr_zone >= 7)
    {
        // 分配间接块
        int second_block = alloc_imap_bit(drive);
        new_inode.zone[7] = second_block;
        for(int i=7;i<nr_zone;i++)
        {
            uint16_t zone = alloc_imap_bit(drive);
            cache_write(drive, second_block, 2, (i-7)*2, &zone);
        }
    }
    cache_write(drive, zone_for_ino, sizeof(inode_disk), bias, &new_inode);
    return ino;
}

/*===========================================================================*
 *				inode_remove				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 释放inode和它指向的文件
 * 执行进程 TASK_FS
 *****************************************************************************/
public void inode_remove(inode_t * inode)
{
    // 释放占用的所有空间
    if(inode->zone[7] != 0)
    {
        // 如果有间接块
        uint16_t nr_data = 1;
        cache_read(inode->dev, inode->zone[7], 2, 0, &nr_data);
        int t = 2;
        while(nr_data)
        {
            free_zmap_bit(inode->dev, nr_data);
            cache_read(inode->dev, inode->zone[7], 2, t, &nr_data);
            t += 2;
        }
        free_zmap_bit(inode->dev, inode->zone[7]);
        inode->zone[7] = 0;
    }
    int nr_zone = 0;
    while(inode->zone[nr_zone] != 0)
    {//释放所有间接块
        free_zmap_bit(inode->dev, inode->zone[nr_zone]);
        nr_zone++;
    }

    free_imap_bit(inode->dev, inode->ino); // 释放自己
}

/*===========================================================================*
 *				inode_sync				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 将inode的修改写回
 * 执行进程 TASK_FS
 *****************************************************************************/
public void inode_sync(inode_t * inode) // 将修改写回磁盘
{
    int inode_per_zone = ZONE_SIZE / sizeof(inode_disk);      // 每个zone中有多少inode                                               
    int zone_nr = 2 + super.imap_blk + super.zmap_blk + (inode->ino - 1) / inode_per_zone; // ino所在的zone号
    int bias = ((inode->ino - 1) % inode_per_zone) * sizeof(inode_disk);// inode在这个zone中的偏移
    cache_write(inode->dev, zone_nr, sizeof(inode_disk), bias, inode); // 写回inode
}

/*===========================================================================*
 *				inode_close				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关闭inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public int inode_get_allocated_size(inode_t * inode)
{
    int zone_nr = 0;
    if(inode->zone[7] != 0) // 如果分配了间接块，那么直接快一定都分配了
    {
        zone_nr = 6;
        uint16_t nr_data = 0;
        do
        {
            zone_nr++; // 8
            cache_read(inode->dev, inode->zone[7],2, 2*(zone_nr-7), &nr_data);
            
        }while(nr_data != 0);
        return zone_nr * ZONE_SIZE;
    }
    while(inode->zone[zone_nr]!=0) zone_nr++;
    return zone_nr * ZONE_SIZE;
}

/*===========================================================================*
 *				inode_close				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关闭inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public int get_inode_by_name(int drive, const char * fname)
{
    inode_t * dir_inode = inode_open(drive, 1); // 首先打开根目录
    directory dir_item;
    char name_cpy[256];
    strcpy(name_cpy, fname);
    char *st = name_cpy + 1;

    while(1)
    {
        char * ed = st;
        while(*ed != '\0' && *ed != '/') ed++;
        if(*ed == '\0') break;
        *ed = '\0';

        int nr_dir_items = dir_inode->size / sizeof(directory);
        int ino = -1;
        for(int i=0;i<nr_dir_items;i++)
        {
            inode_read(dir_inode, &dir_item, sizeof(directory), i * sizeof(directory));
            if(dir_item.ino != 0 && strcmp(dir_item.name, st) == 0)
            {
                ino = dir_item.ino;
                break;
            }
        }
        inode_close(dir_inode);
        if(ino == -1) return -1; // 找不到
        dir_inode = inode_open(drive, ino);
        if((dir_inode->mode & 0x4000) == 0) return inode_close(dir_inode), -1; // 不是目录

        st = ed+1;
    }
    
    int nr_dir_items = dir_inode->size / sizeof(directory);
    int ino = -1;
    for(int i=0;i<nr_dir_items;i++)
    {
        inode_read(dir_inode, &dir_item, sizeof(directory), i * sizeof(directory));
        if(dir_item.ino != 0 && strcmp(dir_item.name, st) == 0)
        {
            ino = dir_item.ino;
            break;
        }
    }
    inode_close(dir_inode);
    return ino;
}

/*===========================================================================*
 *				inode_close				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关闭inode
 * 执行进程 TASK_FS
 *****************************************************************************/
public int get_parent_inode_by_name(int drive, const char * fname)
{
    char fname_cpy[256];
    strcpy(fname_cpy, fname);

    int len = strlen(fname);    
    char * s1 = fname_cpy + len - 1;
    int parent_ino = -1;
    while(s1 > fname_cpy && *s1 != '/') s1--;
    *s1 = '\0';
    if(s1 == fname_cpy) // 父目录是根目录
        parent_ino = 1;
    else parent_ino = get_inode_by_name(0, fname_cpy);
    return parent_ino;
}