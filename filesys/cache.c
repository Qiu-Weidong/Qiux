#include "cache.h"
#include "../kernel/ipc.h"
#include "const.h"
#include "string.h"
#include "stdio.h"

#define CACHE_SIZE 1024
#define CACHE_CNT 128
#define CACHE_INVALID 0
#define CACHE_BUSY 1
#define CACHE_DIRTY 2
// 私有类型定义
typedef struct CACHE
{
    uint16_t flag; // 三种状态 busy、dirty、invalid
    uint16_t dev;  // 设备号
    uint32_t zone_no; // zone号，从0开始映射
    void * addr; // 缓存的内容
} cache_t;

// 私有属性定义
private uint8_t cache_data[CACHE_CNT * CACHE_SIZE];
private cache_t caches[CACHE_CNT];

private void cache_load(int drive, int zone_nr);

/*===========================================================================*
 *				cache_init				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 初始化文件缓冲区
 * 执行进程 TASK_FS
 *****************************************************************************/
public void cache_init()
{
    for(int i=0;i<CACHE_CNT;i++)
    {
        caches[i].addr = (void *)(CACHE_SIZE * i + cache_data);
        caches[i].flag = CACHE_INVALID;
    }
}

/*===========================================================================*
 *				cache_read				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 从文件缓冲区读取数据
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t cache_read(int drive, int zone_nr, size_t size, offset_t offset, void * buffer)
{
    if(offset >= ZONE_SIZE) return 0;
    else if(offset + size > ZONE_SIZE) size = ZONE_SIZE - offset;
    int index = zone_nr % CACHE_CNT;
    cache_load(drive, zone_nr);
    memcpy(buffer, caches[index].addr+offset, size);
    return size;
}

/*===========================================================================*
 *				cache_write				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 向文件缓冲区中写数据
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t cache_write(int drive, int zone_nr, size_t size, offset_t offset, void * buffer)
{
    if(offset >= ZONE_SIZE) return 0;
    else if(offset + size > ZONE_SIZE) size = ZONE_SIZE - offset;
    int index = zone_nr % CACHE_CNT;
    cache_load(drive, zone_nr);
    memcpy(caches[index].addr+offset, buffer, size);
    caches[index].flag = CACHE_DIRTY;
    return size;
}

/*===========================================================================*
 *				cache_sync				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 将缓冲区中的数据写回磁盘
 * 执行进程 TASK_FS
 *****************************************************************************/
public void cache_sync()
{
    for(int i=0;i<CACHE_CNT;i++)
    {
        if(caches[i].flag == CACHE_DIRTY)
        {
            message_t msg;
            msg.type = DEV_WRITE;
            msg.param1 = caches[i].dev;
            msg.param2 = caches[i].zone_no << 1;
            msg.param3 = 2;
            msg.param4 = (int)caches[i].addr;
            if(send(TASK_IDE, &msg) == SEND_SUCCESS) receive(TASK_IDE, &msg);
            caches[i].flag = CACHE_BUSY;
        }
    }
}

// 私有函数定义
private void cache_load(int drive, int zone_nr)
{
    int index = zone_nr % CACHE_CNT;
    if(caches[index].flag == CACHE_INVALID || caches[index].dev != drive 
        || caches[index].zone_no != zone_nr) // 缓存未命中
    {
        message_t msg;
        if(caches[index].flag == CACHE_DIRTY)
        {
            // 写回内存
            msg.type = DEV_WRITE;
            msg.param1 = caches[index].dev;
            msg.param2 = caches[index].zone_no << 1;
            msg.param3 = 2;
            msg.param4 = (int)caches[index].addr;
            if(send(TASK_IDE, &msg) == SEND_SUCCESS) receive(TASK_IDE, &msg);
        }
        // 加载需要的块
        msg.type = DEV_READ;
        msg.param1 = drive;
        msg.param2 = zone_nr << 1;
        msg.param3 = 2;
        msg.param4 = (int)caches[index].addr;
        if(send(TASK_IDE, &msg) == SEND_SUCCESS) receive(TASK_IDE, &msg);
        caches[index].flag = CACHE_BUSY;
        caches[index].dev = drive;
        caches[index].zone_no = zone_nr;
        // putchar('*');   // 缓存未命中，打印一个*出来
    }
}
