#ifndef QIUX_BITMAP_H_
#define QIUX_BITMAP_H_
#include <stdio.h>


int alloc_zmap_bit(FILE * disk);
void free_zmap_bit(FILE * disk, int nr_zone);

int alloc_imap_bit(FILE * disk);
int free_imap_bit(FILE * disk, int ino);

#endif // QIUX_BITMAP_H_