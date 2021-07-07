#ifndef QIUX_SYSCALL_H_
#define QIUX_SYSCALL_H_
#include "type.h"


public void halt();
public void reboot();

public filedesc_t fopen(const char * fname, uint32_t mode);
public void fclose(filedesc_t fd);
public size_t fread(filedesc_t fd, const char * buffer, size_t n);
public size_t fwrite(filedesc_t fd, const char * buffer, size_t n);
public uint32_t ftell(filedesc_t fd);
public void fseek(filedesc_t fd, int off, int mode);
public void mkdir(const char * dirname);
public void fcreate(const char * fname);
public void fremove(const char * fname);
public void fsync();

public pid_t exec(const char * path, const char * argv[]);
public void exit(int status);
public int wait(pid_t pid);
#endif // QIUX_SYSCALL_H_
