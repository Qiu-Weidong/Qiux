#ifndef QIUX_ATOMIC_H_
#define QIUX_ATOMIC_H_

/*
 * 常用的原子操作，用于实现进程间的互斥
*/

typedef struct ATOMIC
{
    volatile int value;
} atomic_t;

static inline void atomic_add(atomic_t *v, int i)
{
    __asm__ volatile("lock addl %1,%0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_sub(atomic_t *v, int i)
{
    __asm__ volatile("lock subl %1, %0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_inc(atomic_t *v)
{
    __asm__ volatile("lock incl %0"
                     : "+m"(v->value));
}

static inline void atomic_dec(atomic_t *v)
{
    __asm__ volatile("lock decl %0"
                     : "+m"(v->value));
}

static inline int atomic_get(atomic_t *v)
{
    return *(volatile int *)&v->value;
}

static inline void atomic_set(atomic_t *v, int i)
{
    *(volatile int *)&v->value = i;
}

static inline int test_and_set(atomic_t *lock)
{
    // 用1和v中的值进行交换，并返回v中的内容
    int _v = 1;
    __asm__ volatile("lock xchg %1, %0"
                     : "+m"(lock->value), "+r"(_v)::);
    return _v;
}

static inline void atomic_clear(atomic_t *v)
{
    __asm__ volatile("lock andl $0, %0"
                     : "+m"(v->value));
}

static inline void atomic_adc(atomic_t *v, int i)
{
    __asm__ volatile("lock adc %1, %0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_and(atomic_t *v, int i)
{
    __asm__ volatile("lock andl %1, %0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_or(atomic_t *v, int i)
{
    __asm__ volatile("lock orl %1, %0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_xor(atomic_t *v, int i)
{
    __asm__ volatile("lock xorl %1, %0"
                     : "+m"(v->value)
                     : "ir"(i));
}

static inline void atomic_not(atomic_t *v)
{
    // 将每一位取反
    __asm__ volatile("lock notl %0"
                     : "+m"(v->value));
}

static inline void atomic_neg(atomic_t *v)
{
    // 取反加一
    __asm__ volatile("lock negl %0"
                     : "+m"(v->value));
}

#endif // QIUX_ATOMIC_H_