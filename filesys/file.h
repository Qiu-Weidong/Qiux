#ifndef QIUX_FILE_H_
#define QIUX_FILE_H_
#include "type.h"
#include "inode.h"
#include "list.h"


typedef struct FILE
{
    uint32_t fd;
    uint32_t mode;
    uint32_t pos;
    inode_t * inode;

    list_elem elem;
    list_elem slot_elem;
} file_t;

public void file_init();
public file_t * file_open(inode_t * inode, uint32_t mode);
public void file_close(file_t * file);
public size_t file_read(file_t * file, void * buffer, size_t size);
public size_t file_write(file_t *file, void *buffer, size_t size);
public size_t file_size(file_t * file);
public void file_seek(file_t *file, offset_t off, int mode);
public offset_t file_tell(file_t *file);

#endif // QIUX_FILE_H_

