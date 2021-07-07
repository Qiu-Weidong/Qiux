#ifndef QIUX_CACHE_H_
#define QIUX_CACHE_H_
#include "type.h"

public void cache_init();

public size_t cache_read(int drive, int zone_nr, size_t size, offset_t offset, void * buffer);
public size_t cache_write(int drive, int zone_nr, size_t size, offset_t offset, void * buffer);
public void cache_sync();
#endif // QIUX_CACHE_H_

