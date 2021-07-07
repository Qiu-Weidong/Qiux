#ifndef QIUX_CLOCK_H_
#define QIUX_CLOCK_H_
#include "type.h"

#define PERIOD 54757036
#define FREQ   18
#define TIMER0         0x40 
#define TIMER_MODE     0x43 
#define RATE_GENERATOR 0x34

public void clock_init();
public void clock_handler(const intr_frame * frame, const process_t * proc);
#endif // QIUX_CLOCK_H_

