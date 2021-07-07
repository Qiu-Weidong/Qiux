#ifndef QIUX_IDE_H_
#define QIUX_IDE_H_
#include "type.h"

public void ide_init();
public void ide_identify(int drive);
public void ide_read(int drive, int sector_nr, int n, void * buf);
public void ide_write(int drive, int sector_nr, int n, void * buf);
public void ide_handler(const intr_frame * frame UNUSED, process_t * p_proc UNUSED);


#endif // QIUX_IDE_H_
