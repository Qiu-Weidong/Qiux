#ifndef QIUX_PAGE_H_
#define QIUX_PAGE_H_
#include "type.h"


public void page_init();
public void * palloc(int nr_pages); // 分配连续的nr_pages个页
public void pfree(void * addr, int nr_pages);          // 释放连续的nr_pages个页，从addr开始

#endif // QIUX_PAGE_H_


