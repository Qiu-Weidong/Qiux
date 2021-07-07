#ifndef QIUX_INODE_H_
#define QIUX_INODE_H_
#include "type.h"
#include "list.h"

typedef struct INODE_DISK
{
    uint16_t mode;  // 文件类型和 RWX 访问控制位
    uint16_t uid;   // 文件属主的用户 ID            (unused)
    uint32_t size;  // 文件大小, 以 byte 计数
    uint32_t mtime; // 自从 1970.1.1 以来的秒数     (unused)
    uint8_t gid;    // 文件属主 所属的组            (unused)
    uint8_t nlinks; // 该节点被多少个目录所链接

    /*
     * zone[0] - zone[6] 分别指向 7 个直接块
     * zone[7] 指向间接块
     * zone[8] 指向双重间接块                       (unused)
     */
    uint16_t zone[9];
}inode_disk;

typedef struct INODE
{
    /*
     * fedc ba98 7654 3210
     * rdcf  gur wxrw xrwx
     * 0100 0001 1110 1101  
     * 1000 0001 1010 0100 
     * 
     * 0 ~ 2 : 其他人rxw
     * 3 ~ 5 : 同组rxw
     * 6 ~ 8 : 创建者rxw
     * 9 执行时设置uid
     * a 执行时设置gid
     * b 不清楚
     * c FIFO文件
     * d 字符设备文件
     * e 目录文件
     * f 常规文件
    */
    uint16_t mode;  // mode ， 偏移 0
    uint16_t uid;   // (unused)偏移 2
    uint32_t size;  // 偏移 4
    uint32_t mtime; // (unused) 8
    uint8_t gid;    // (unused) 12
    uint8_t nlinks; // 13 
    uint16_t zone[9]; // 32字节 zone[0] 

    
    uint32_t ino;   // i 节点号码，注意从1开始
    uint16_t open_cnt;   // 内存引用计数
    uint16_t dev;   // i 节点所在的磁盘, Qiux 只支持单个磁盘, 所以始终为 0
    list_elem slot_hash_elem; 
} inode_t;// 48字节

public void inode_init();
public inode_t * inode_open(int drive, int ino);
public void inode_close(inode_t * );
public size_t inode_read(inode_t * , void * buffer, size_t size, offset_t offset);
public size_t inode_write(inode_t *, void * buffer, size_t size, offset_t offset);
public size_t inode_trunc(inode_t *inode, offset_t offset);

public int inode_create(int drive, uint16_t mode, uint16_t nr_zone); // 预分配nr_zone个zone
public void inode_remove(inode_t * inode);
public void inode_sync(inode_t * inode);
public int inode_get_allocated_size(inode_t * );
public int get_inode_by_name(int drive, const char * fname);
public int get_parent_inode_by_name(int drive, const char * fname);
#endif // QIUX_INODE_H_
