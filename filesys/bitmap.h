#ifndef QIUX_BITMAP_H_
#define QIUX_BITMAP_H_
#include "type.h"

public int alloc_zmap_bit(int drive);
public void free_zmap_bit(int drive, int nr_zone);

public int alloc_imap_bit(int drive);
public int free_imap_bit(int drive, int ino);

#endif // QIUX_BITMAP_H_