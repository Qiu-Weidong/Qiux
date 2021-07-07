#ifndef QIUOS_DESC_H
#define QIUOS_DESC_H
#include "type.h"
#include "const.h"

public uint64_t make_desc(uint32_t base, uint32_t limit, uint16_t attr);
public uint64_t make_seg_desc(uint32_t base, uint32_t limit, uint16_t attr);
public uint64_t make_ldt_desc(uint32_t base, uint32_t limit, uint16_t attr);
public uint64_t make_tss_desc(uint32_t base, uint32_t limit, uint16_t attr);

public uint64_t make_gate(void (* func)(), selector_t selector, uint8_t dcount, uint8_t attr);

public uint64_t make_call_gate(void (* func)() , selector_t selector, uint8_t dcount, uint8_t dpl);
public uint64_t make_trap_gate(void (* func)() , selector_t selector, uint8_t dpl);
public uint64_t make_intr_gate(void (* func)() , selector_t selector, uint8_t dpl);
public uint64_t make_task_gate(selector_t selector, uint8_t dpl);


public void * get_base_from_desc(uint64_t );
public uint32_t get_limit_from_desc(uint64_t );
public uint16_t get_attr_from_desc(uint64_t );
public void * get_fun_from_gate(uint64_t );
public uint8_t get_attr_from_gate(uint64_t );
public selector_t get_selector_from_gate(uint64_t );

#endif // QIUOS_DESC_H

