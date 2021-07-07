#ifndef QIUX_TTY_H_
#define QIUX_TTY_H_
#include "type.h"

public void tty_dev_read_cur();
public void tty_dev_write_all();
public void tty_init();
public void tty_read(void * buffer, size_t size, pid_t pid);
public void tty_write(void * buffer, size_t size, pid_t pid);
public void tty_switch(int idx);
#endif // QIUX_TTY_H_


