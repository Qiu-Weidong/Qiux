#ifndef QIUX_FILESYS_H_
#define QIUX_FILESYS_H_
#include "type.h"

public uint32_t filesys_open(const char * fname, uint32_t mode, pid_t pid);
public int filesys_close(filedesc_t fd, pid_t pid);
public size_t filesys_read(filedesc_t fd, void * buffer, size_t size, pid_t pid);
public size_t filesys_write(filedesc_t fd, void * buffer, size_t size, pid_t pid);
public void filesys_seek(filedesc_t fd, int off, int mode, pid_t pid);
public uint32_t filesys_tell(filedesc_t fd, pid_t pid);
public int filesys_create(const char * fname, uint16_t mode);
public int filesys_remove(const char * fname);
public int filesys_rmdir(const char * dname);

#endif // QIUX_FILESYS_H_


