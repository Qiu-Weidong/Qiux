#ifndef QIUX_SUPER_H_
// #include "type.h"


typedef struct SUPER_BLOCK
{
    unsigned short ninodes;       // inodes的个数
    unsigned short nzones;        // zones的个数(一个zone占用两个扇区)
    unsigned short imap_blk;      // i 节点位图 占用块的数目
    unsigned short zmap_blk;      // 数据块位图 占用的块的数目
    unsigned short fst_data_zone; // 第一个 数据块 的块号
    unsigned short log_zone_size; // 一个虚拟块的大小 = 1024 << log_zone_size

    unsigned int max_size; // 能存放的最大文件大小(以 byte 计数)
    unsigned short magic;    // magic number
    unsigned short state;    // 状态，值为1
} super_block;
#endif // QIUX_SUPER_H_

