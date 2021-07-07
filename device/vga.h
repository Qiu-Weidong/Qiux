#ifndef QIUX_VGA_H_
#include "type.h"


public void set_cursor(uint32_t position);

public void set_video_start_addr(uint32_t addr);

public void vga_flush(uint32_t position, uint32_t addr);

#endif // QIUX_VGA_H_

