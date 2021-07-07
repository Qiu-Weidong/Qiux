#include "../kernel/ipc.h"
#include "const.h"
#include "stdio.h"
#include "debug.h"
#include "inode.h"
#include "directory.h"
#include "../include/string.h"
#include "cache.h"
#include "super.h"
#include "file.h"
#include "bitmap.h"
#include "../kernel/syscall.h"
#include "../kernel/proc.h"

public uint8_t fs_buf[ZONE_SIZE]; // 用于各个地方作为缓存
public super_block super;


/*===========================================================================*
 *				fname2ino				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 通过完整的路径查找到ino, 没找到返回-1
 * 执行进程 TASK_FS
 *****************************************************************************/
private uint32_t fname2ino(int drive, const char * fname)
{
    directory dir_item;
    char name_cpy[64];
    strcpy(name_cpy, fname);

    char *st = name_cpy + 1; // 跳过根目录 '/'
    
    char * flag = st;
    while(*flag != '\0') flag++;
    
    uint16_t parent_ino = 0, ino = 1;
    inode_t * dir_inode = inode_open(drive, ino); // 首先打开根目录
    while(st < flag)
    {
        parent_ino = ino;
        if((dir_inode->mode & 0x4000) == 0) return inode_close(dir_inode), -1; // 不是目录
        char * ed = st;
        while(*ed != '\0' && *ed != '/') ed++;
        *ed = '\0';
        int nr_dir_items = dir_inode->size / sizeof(directory); // 根目录下的目录项条目数

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
        if(ino == parent_ino) return -1; // 没找到
        dir_inode = inode_open(drive, ino);
        st = ed + 1;
    }    
    inode_close(dir_inode);
    return ino | (parent_ino << 16);
}

/*===========================================================================*
 *				filesys_open				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 打开文件, 如果成功则返回文件描述符，否则返回-1
 * 执行进程 TASK_FS
 *****************************************************************************/
public uint32_t filesys_open(const char * fname, uint32_t mode, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    int ino = fname2ino(0, fname) & 0xffff;
    if(ino <= 0) return -1;

    inode_t * inode = inode_open(0, ino);
    if(inode == nullptr) return -1;
    file_t * file = file_open(inode, mode);
    if(file == nullptr) return -1;
    list_push_back(&sender->open_files, &file->elem);
    return file->fd;
}

/*===========================================================================*
 *				filesys_close				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 关闭文件，成功关闭返回0
 * 执行进程 TASK_FS
 *****************************************************************************/
public int filesys_close(filedesc_t fd, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    for(list_elem * e = list_begin(&sender->open_files); e != list_end(&sender->open_files);
        e = list_next(e))
    {
        file_t * file = list_entry(e, file_t, elem);
        if(file->fd == fd)
        {
            file_close(file);
            list_remove(e);
            return 0;
        }
    }
    return -1;
}

/*===========================================================================*
 *				filesys_read				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 读文件
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t filesys_read(filedesc_t fd, void * buffer, size_t size, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    // 在进程的文件列表中查找fd文件
    // 调用file_read读取文件
    for(list_elem * e = list_begin(&sender->open_files); e != list_end(&sender->open_files);
        e = list_next(e))
    {
        file_t * file = list_entry(e, file_t, elem);
        if(file->fd == fd)
        {
            return file_read(file, buffer, size);
        }
    }
    return 0;
}

/*===========================================================================*
 *				filesys_write				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 写文件
 * 执行进程 TASK_FS
 *****************************************************************************/
public size_t filesys_write(filedesc_t fd, void * buffer, size_t size, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    // 在进程的文件列表中查找fd文件
    // 调用file_write写文件
    for(list_elem * e = list_begin(&sender->open_files); e != list_end(&sender->open_files);
        e = list_next(e))
    {
        file_t * file = list_entry(e, file_t, elem);
        if(file->fd == fd)
        {
            return file_write(file, buffer, size);
        }
    }
    return 0;
}

/*===========================================================================*
 *				filesys_seek				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 移动文件pos
 * 执行进程 TASK_FS
 *****************************************************************************/
public void filesys_seek(filedesc_t fd, int off, int mode, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    // 在进程的文件列表中查找fd文件
    // 调用file_seek
    for(list_elem * e = list_begin(&sender->open_files); e != list_end(&sender->open_files);
        e = list_next(e))
    {
        file_t * file = list_entry(e, file_t, elem);
        if(file->fd == fd)
        {
            file_seek(file, off, mode);
        }
    }
}

/*===========================================================================*
 *				filesys_tell				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 返回文件的pos，出错返回-1
 * 执行进程 TASK_FS
 *****************************************************************************/
public uint32_t filesys_tell(filedesc_t fd, pid_t pid)
{
    process_t * sender = pid2proc(pid);
    assert(sender != nullptr);

    // 在进程的文件列表中查找fd文件
    // 调用file_tell
    for(list_elem * e = list_begin(&sender->open_files); e != list_end(&sender->open_files);
        e = list_next(e))
    {
        file_t * file = list_entry(e, file_t, elem);
        if(file->fd == fd)
        {
            return file_tell(file);
        }
    }
    return -1;
}

private const char * get_file_name(const char * path)
{
    int len = strlen(path);
    const char * result = path + len - 1;
    while(result > path && *result != '/') result--;
    if(*result == '/') result++;
    return result;
}

/*===========================================================================*
 *				filesys_create				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 创建文件
 * 执行进程 TASK_FS
 *****************************************************************************/
public int filesys_create(const char * fname, uint16_t mode)
{
    // 首先解析出文件名和路径
    const char * s1 = get_file_name(fname);
    char name_cpy[16] ; name_cpy[14] = '\0';
    strncpy(name_cpy, s1, 14); // 文件名不能够超过14
    s1--;
    if(*s1 != '/') return -1; // 路径不合法

    char path_cpy[64];
    strncpy(path_cpy, fname, s1-fname);
    path_cpy[s1-fname] = '\0';

    // 在path_cpy路径下创建name_cpy文件
    int parent_ino = fname2ino(0, path_cpy) & 0xffff;
    if(parent_ino <= 0) return -1;

    inode_t * parent = inode_open(0, parent_ino);
    if(parent == nullptr || (parent->mode & 0x4000) == 0) return -1; // 不是目录

    directory dir;
    
    int n = parent->size / sizeof(directory);
    for(int i=0;i<n;i++)
    {
        inode_read(parent, &dir, sizeof(directory), i*sizeof(directory));
        if(dir.ino != 0 && strcmp(dir.name, name_cpy) == 0)
        {
            return -1;
        }
    }
    
    int ino = inode_create(0, mode, 1);
    for(int i=0;i<n;i++)
    {
        inode_read(parent, &dir, sizeof(directory), i*sizeof(directory));
        if(dir.ino == 0)
        {
            dir.ino = ino;
            strncpy(dir.name, name_cpy, 14);
            inode_write(parent, &dir, sizeof(directory), i*sizeof(directory));
            goto ok;
        }
    }
    dir.ino = ino;
    strncpy(dir.name, name_cpy, 14);
    inode_write(parent, &dir, sizeof(directory), parent->size);
ok:

    if(mode & 0x4000) // 如果是目录, 则在其中创建 . 和 .. 两个目录项
    {
        strcpy(dir.name, ".");
        inode_t * self = inode_open(0, ino);
        inode_write(self, &dir, sizeof(directory), self->size);
        dir.ino = parent_ino;
        strcpy(dir.name, "..");
        inode_write(self, &dir, sizeof(directory), self->size);
        
        self->nlinks++;
        parent->nlinks++;
        inode_sync(self);
        inode_sync(parent);
        inode_close(self);
    }
    inode_close(parent);

}

private void remove_dir(inode_t * inode)
{
    directory dir;
    int nr_dirs = inode->size / sizeof(directory);
    for(int i=0;i<nr_dirs;i++)
    {
        inode_read(inode, &dir, sizeof(directory), i*sizeof(directory));
        if(dir.ino != 0 && strcmp(dir.name, ".")!=0 && strcmp(dir.name, "..") != 0)
        {
            inode_t * child = inode_open(0, dir.ino);
            if(child->mode & 0x4000) remove_dir(child);
            else inode_remove(child);
            inode_close(child);
        }
        
    }
    inode_remove(inode);
}

/*===========================================================================*
 *				filesys_remove				     *
 *===========================================================================*/
/*****************************************************************************
 * <特权级 1> 删除文件
 * 执行进程 TASK_FS
 *****************************************************************************/
public int filesys_remove(const char * fname)
{
    int ino = fname2ino(0, fname);
    if(ino == -1) return -1;
    int parent_ino = (uint32_t)ino >> 16;
    ino &= 0xffff;

    const char * name = get_file_name(fname);

    inode_t * parent = inode_open(0, parent_ino);
    inode_t * inode = inode_open(0, ino);
    if(inode->mode & 0x4000) // 如果是目录，则删除所有子目录和文件
    {
        remove_dir(inode); 
        parent->nlinks--;    // 目录有一个指向上一级的链接
        inode_sync(parent);
    }
    else 
        inode_remove(inode);
    inode_close(inode);

    // 然后在父级目录下删除相应的条目
    directory dir;
    int nr_dirs = parent->size / sizeof(directory);
    for(int i=0;i<nr_dirs;i++)
    {
        inode_read(parent, &dir, sizeof(directory), i*sizeof(directory));
        if(dir.ino != 0 && strcmp(dir.name, name) == 0)
        {
            dir.ino = 0;
            strcpy(dir.name, "WEST.TMP");
            inode_write(parent, &dir, sizeof(directory), i*sizeof(directory));
            break;
        }
    }
}



