#ifndef QIUX_INODE_H_
#define QIUX_INODE_H_
#include <stdio.h>


/*
Minix v1 file system structure
zone: 
0            1            2                  2 + imap_blk      2+imap_blk+zmap_blk    fst_data_zone
+-------------------------------------------------------------------------------------------------+
| bootsector | superblock | inode bitmap ... | zone bitmap ... | inodes zone          | data zone |
+-------------------------------------------------------------------------------------------------+
1 zone = 2 block = 1024 byte
*/
typedef struct INODE
{
    unsigned short mode;  // 文件类型和 RWX 访问控制位
    unsigned short uid;   // 文件属主的用户 ID            (unused)
    unsigned int size;  // 文件大小, 以 byte 计数
    unsigned int mtime; // 自从 1970.1.1 以来的秒数     (unused)
    unsigned char gid;    // 文件属主 所属的组            (unused)
    unsigned char nlinks; // 该节点被多少个目录所链接

    /*
     * zone[0] - zone[6] 分别指向 7 个直接块
     * zone[7] 指向间接块
     * zone[8] 指向双重间接块                       (unused)
     */
    unsigned short zone[9];
}inode;


#endif // QIUX_INODE_H_
