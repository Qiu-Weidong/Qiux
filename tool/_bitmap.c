#include "bitmap.h"
#include "super.h"
/*
Minix v1 file system structure
zone: 
0            1            2                  2 + imap_blk      2+imap_blk+zmap_blk    fst_data_zone
+-------------------------------------------------------------------------------------------------+
| bootsector | superblock | inode bitmap ... | zone bitmap ... | inodes zone          | data zone |
+-------------------------------------------------------------------------------------------------+
1 zone = 2 block = 1024 byte
*/
#define ZONE_SIZE 1024
extern super_block super;
int alloc_bit(FILE * disk, int zone_st, int nr_zone);
int alloc_zmap_bit(FILE * disk) // 分配zone, 返回分配的zone号
{
    fseek(disk, ZONE_SIZE * (2 + super.imap_blk), SEEK_SET);
    int n = ZONE_SIZE * super.zmap_blk; // 总大小，单位字节
    for(int i=0;i<n;i++)
    {
        unsigned char c;
        fread(&c, 1, 1, disk);
        if(c != 0xff)
        {
            int bias = i << 3;
            unsigned char mask = ~c;
            unsigned char t = 1;
            while((t & mask) == 0)
            {
                t <<= 1;
                bias++;
            }
            c |= t;
            fseek(disk, ZONE_SIZE * (2 + super.imap_blk) + bias / 8, SEEK_SET);
            fwrite(&c, 1, 1, disk);
            return bias - 1 + super.fst_data_zone;
        }
    }
}
void free_zmap_bit(FILE * disk, int nr_zone)
{
    nr_zone -= super.fst_data_zone;
    nr_zone += 1; // 加一是因为第一个bit未使用
    int nr_byte = nr_zone / 8 + ZONE_SIZE *(2 + super.imap_blk); 
    int mask = ~(1 << (nr_zone % 8)); // 位偏移
    fseek(disk, nr_byte, SEEK_SET);
    unsigned char c;
    fread(disk, 1, 1, &c);
    c &= mask;
    fseek(disk, nr_byte, SEEK_SET);
    fwrite(disk, 1, 1, &c);
}

int alloc_imap_bit(FILE * disk)  // 返回ino
{
    fseek(disk, ZONE_SIZE * 2, SEEK_SET);
    int n = ZONE_SIZE * super.imap_blk;
    for(int i=0;i<n;i++)
    {
        unsigned char c;
        fread(&c, 1, 1, disk);
        if(c != 0xff)
        {
            int bias = i << 3;
            unsigned char mask = ~c;
            unsigned char t = 1;
            while((t & mask) == 0)
            {
                t <<= 1;
                bias++;
            }
            c |= t;
            fseek(disk, ZONE_SIZE * 2 + bias / 8, SEEK_SET);
            fwrite(&c, 1, 1, disk);
            return bias;
        }

    }
    return alloc_bit(disk, 2, super.imap_blk) + 1;
}
int free_imap_bit(FILE * disk, int ino) // 注意ino从1开始
{
    int nr_byte = ino / 8 + 2 * ZONE_SIZE;
    int mask = ~(1 << (ino % 8));

    fseek(disk, nr_byte, SEEK_SET);
    unsigned char c;
    fread(disk, 1, 1, &c);
    c &= mask;
    fseek(disk, nr_byte, SEEK_SET);
    fwrite(disk, 1, 1, &c);
}

int alloc_bit(FILE * disk, int zone_st, int nr_zone)
{
    for(int i=0;i<nr_zone;i++) // 遍历zone bitmap
    {
        // cache_read(disk, i+zone_st, ZONE_SIZE, 0, bitmap_buf);
        for(int j=0;j<ZONE_SIZE;j++)
        {
            // if(bitmap_buf[j] != 0xff) // 如果有可以用的块
            // {
                // char mask = ~bitmap_buf[j]; // 取反后，可用的位为1
                char mask;
                int result = (i * 1024 + j) * 8;
                int t = 1;
                while((t & mask) == 0)
                {
                    t <<= 1;
                    result++;
                }
                // char c =  bitmap_buf[j] | t; // 将这一位置为1
                // cache_write(disk, i+zone_st, 1, j, &c); // 写回
                return result-1; // 减去1是因为第一个bit不使用
            // }
        }
    }

}
