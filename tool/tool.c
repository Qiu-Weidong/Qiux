#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "super.h"
#include "bitmap.h"
#include "inode.h"

/*
Minix v1 file system structure
zone: 
0            1            2                  2 + imap_blk      2+imap_blk+zmap_blk    fst_data_zone
+-------------------------------------------------------------------------------------------------+
| bootsector | superblock | inode bitmap ... | zone bitmap ... | inodes zone          | data zone |
+-------------------------------------------------------------------------------------------------+
1 zone = 2 block = 1024 byte
*/
super_block super;
const int zone_size = 1024;
typedef struct DIRECTORY
{
    unsigned short ino;
    char name[14];
} directory;

unsigned short second[512];
char buffer[1024];

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        return puts("too little params!"), 0;
    }
    FILE *disk = fopen(argv[argc - 1], "r+b");
    if (disk == NULL)
    {
        return printf("can\'t open disk %s\n", argv[2]), 0;
    }

    // 读取super
    fseek(disk, zone_size, SEEK_SET);
    fread(&super, sizeof(super_block), 1, disk);

    for (int i = 1; i < argc - 1; i++)
    {
        FILE *f = fopen(argv[i], "rb");
        if (f == NULL)
        {
            return printf("can\'t open file %s\n", argv[i]), 0;
        }

        int n = strlen(argv[i]);
        char *filename = argv[i] + n - 1;
        while (filename > argv[i] && *filename != '/')
            filename--;
        if (*filename == '/')
            filename++;

        int ino;
        inode root_ino;
        int inode_bias;

        inode_bias = zone_size * (2 + super.imap_blk + super.zmap_blk);
        fseek(disk, inode_bias, SEEK_SET);
        fread(&root_ino, sizeof(inode), 1, disk);

        int dir_item_nr = root_ino.size / sizeof(directory);
        fseek(disk, super.fst_data_zone * zone_size, SEEK_SET);
        fread(buffer, zone_size, 1, disk);
        directory *root = (directory *)buffer;
        int flag = 0;
        for (int i = 0; i < dir_item_nr; i++)
        {
            if (strcmp(root[i].name, filename) == 0)
            {
                printf("%s is already here!\n", filename);
                goto done;
            }
        }

        ino = alloc_imap_bit(disk);

        for (int i = 0; i < dir_item_nr; i++)
        {
            if (root[i].ino == 0)
            {
                root[i].ino = ino;
                strcpy(root[i].name, filename);
                flag = 1;
                break;
            }
        }

        if (!flag)
        {
            root_ino.size += sizeof(directory);
            root[dir_item_nr].ino = ino;
            strcpy(root[dir_item_nr].name, filename);

            inode_bias = zone_size * (2 + super.imap_blk + super.zmap_blk);
            fseek(disk, inode_bias, SEEK_SET);
            fwrite(&root_ino, sizeof(inode), 1, disk);
        }

        // 将根目录写回磁盘
        fseek(disk, super.fst_data_zone * zone_size, SEEK_SET);
        fwrite(buffer, zone_size, 1, disk);

        // 计算kernel的大小
        struct stat statbuf;
        stat(argv[1], &statbuf);
        int kernel_size = statbuf.st_size;

        inode kernel_ino;
        memset(&kernel_ino, 0, sizeof(inode));

        kernel_ino.mode = 0x81b4;
        kernel_ino.size = kernel_size;
        kernel_ino.nlinks = 1;

        int nr_zones = (kernel_size + zone_size - 1) / zone_size;
        for (int i = 0; i < nr_zones && i < 7; i++)
        {
            kernel_ino.zone[i] = alloc_zmap_bit(disk);
            fread(buffer, zone_size, 1, f);
            fseek(disk, kernel_ino.zone[i] * zone_size, SEEK_SET);
            fwrite(buffer, zone_size, 1, disk);
        }
        if (nr_zones > 7)
        {
            memset(second, 0, sizeof(second));
            kernel_ino.zone[7] = alloc_zmap_bit(disk);
            for (int i = 7; i < nr_zones; i++)
            {
                second[i - 7] = alloc_zmap_bit(disk);
                fread(buffer, zone_size, 1, f);
                fseek(disk, second[i - 7] * zone_size, SEEK_SET);
                fwrite(buffer, zone_size, 1, disk);
            }
            fseek(disk, kernel_ino.zone[7] * zone_size, SEEK_SET);
            fwrite(second, zone_size, 1, disk);
        }

        // 将kernel的inode写入磁盘
        inode_bias = zone_size * (2 + super.imap_blk + super.zmap_blk) + (ino - 1) * sizeof(inode);
        fseek(disk, inode_bias, SEEK_SET);
        fwrite(&kernel_ino, sizeof(inode), 1, disk);
done:
        fclose(f);
    }

    fclose(disk);
    return 0;
}
