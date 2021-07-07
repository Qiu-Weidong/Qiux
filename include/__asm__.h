#ifndef QIUX_ASM_H_
#define QIUX_ASM_H_
#include "type.h"

// 常用的内嵌汇编语句
// 注意，为避免与int关键字同名，int指令声明为intr
private inline void nop() { __asm__ volatile("sti" ::); }
private inline void sti() { __asm__ volatile("sti" ::); }
private inline void cli() { __asm__ volatile("cli" ::); }
private inline void cld() { __asm__ volatile("cld" ::); }
private inline void std() { __asm__ volatile("std" ::); }
private inline void hlt() { __asm__ volatile("hlt" ::); }
private inline void ud2() { __asm__ volatile("ud2" ::); }
private inline void ret() { __asm__ volatile("ret" ::); }
private inline void iret() { __asm__ volatile("iret" ::); }
private inline void into() { __asm__ volatile("into" ::); }
private inline void bound(void *dest, void *src) { __asm__ volatile("boundl %0, (%1)" ::"r"(dest), "g"(src)); }
private inline void intr(int n) { __asm__ volatile("int %0" ::"g"(n)); }
private inline void int3() { __asm__ volatile("int3" ::); }
private inline void pushf() { __asm__ volatile("pushfl" ::
                                                  : "memory"); }
private inline void popf() { __asm__ volatile("popfl" ::
                                                 : "memory"); }

private inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile("inb %w1, %b0"
                     : "=a"(data)
                     : "Nd"(port));
    return data;
}
private inline void outb(uint16_t port, uint8_t data) { __asm__ volatile("outb %b0, %w1"
                                                                        :
                                                                        : "a"(data), "Nd"(port)); }
private inline uint16_t inw(uint16_t port)
{
    uint16_t data;
    __asm__ volatile("inw %w1, %w0"
                     : "=a"(data)
                     : "Nd"(port));
    return data;
}
private inline void outw(uint16_t port, uint16_t data) { __asm__ volatile("outw %w0, %w1"
                                                                         :
                                                                         : "a"(data), "Nd"(port)); }
private inline uint32_t inl(uint16_t port)
{
    uint32_t data;
    __asm__ volatile("inl %w1, %0"
                     : "=a"(data)
                     : "Nd"(port));
    return data;
}

private inline void outl(uint16_t port, uint32_t data) { __asm__ volatile("outl %0, %w1"
                                                                         :
                                                                         : "a"(data), "Nd"(port)); }
private inline void insb(uint16_t port, void *addr, size_t cnt)
{
    __asm__ volatile("cld;rep insb"
                     : "+D"(addr), "+c"(cnt)
                     : "d"(port)
                     : "memory");
}
private inline void insw(uint16_t port, void *addr, size_t cnt)
{
    __asm__ volatile("cld;rep insw"
                     : "+D"(addr), "+c"(cnt)
                     : "d"(port)
                     : "memory");
}
private inline void insl(uint16_t port, void *addr, size_t cnt)
{
    __asm__ volatile("rep insl"
                     : "+D"(addr), "+c"(cnt)
                     : "d"(port)
                     : "memory");
}

private inline void outsb(uint16_t port, const void *addr, size_t cnt)
{
    __asm__ volatile("cld;rep outsb"
                     : "+S"(addr), "+c"(cnt)
                     : "d"(port));
}
private inline void outsw(uint16_t port, const void *addr, size_t cnt)
{
    __asm__ volatile("cld;rep outsw"
                     : "+S"(addr), "+c"(cnt)
                     : "d"(port));
}
private inline void outsl(uint16_t port, const void *addr, size_t cnt)
{
    __asm__ volatile("rep outsl"
                     : "+S"(addr), "+c"(cnt)
                     : "d"(port));
}

private inline void lgdt(void *gdt_ptr) { __asm__ volatile("lgdt (%0)" ::"r"(gdt_ptr)); }
private inline void lidt(void *idt_ptr) { __asm__ volatile("lidt (%0)" ::"r"(idt_ptr)); }
private inline void ltr(uint16_t tss_sel) { __asm__ volatile("ltr  %w0" ::"r"(tss_sel)); }
private inline void lldt(uint16_t ldt_sel) { __asm__ volatile("lldt %w0" ::"r"(ldt_sel)); }
private inline void sgdt(void *gdt_ptr) { __asm__ volatile("sgdt (%0)" ::"r"(gdt_ptr)
                                                          : "memory"); }
private inline void sidt(void *idt_ptr) { __asm__ volatile("sidt (%0)" ::"r"(idt_ptr)
                                                          : "memory"); }

private inline uint16_t get_ds()
{
    uint16_t _v;
    __asm__ volatile("movw %%ds, %0"
                     : "=r"(_v)
                     :);
    return _v;
}
private inline uint16_t get_es()
{
    uint16_t _v;
    __asm__ volatile("movw %%es, %0"
                     : "=r"(_v)
                     :);
    return _v;
}
private inline uint16_t get_fs()
{
    uint16_t _v;
    __asm__ volatile("movw %%fs, %0"
                     : "=r"(_v)
                     :);
    return _v;
}
private inline uint16_t get_gs()
{
    uint16_t _v;
    __asm__ volatile("movw %%gs, %0"
                     : "=r"(_v)
                     :);
    return _v;
}
private inline uint16_t get_ss()
{
    uint16_t _v;
    __asm__ volatile("movw %%ss, %0"
                     : "=r"(_v)
                     :);
    return _v;
}

private inline void set_ds(uint16_t value) { __asm__ volatile("movw %w0, %%ds" ::"r"(value)); }
private inline void set_es(uint16_t value) { __asm__ volatile("movw %w0, %%es" ::"r"(value)); }
private inline void set_fs(uint16_t value) { __asm__ volatile("movw %w0, %%fs" ::"r"(value)); }
private inline void set_gs(uint16_t value) { __asm__ volatile("movw %w0, %%gs" ::"r"(value)); }
private inline void set_ss(uint16_t value) { __asm__ volatile("movw %w0, %%ss" ::"r"(value)); }
private inline void set_esp(uint32_t value) { __asm__ volatile("movl %0, %%esp" ::"g"(value)); }

private inline uint32_t get_eflags()
{
    uint32_t _v;
    __asm__ volatile("pushfl; popl %0"
                     : "=g"(_v)::"memory");
    return _v;
}
private inline void set_eflags(uint32_t flags) { __asm__ volatile("pushl %0; popf" ::"g"(flags)
                                                                 : "memory"); }

private inline void ljmp(void *fptr) { __asm__ volatile("ljmp *(%0)" ::"g"(fptr)); }
private inline void barrier() { __asm__ volatile(""
                                                :
                                                :
                                                : "memory"); }

#endif // QIUX_ASM_H_
