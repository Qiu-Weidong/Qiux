#ifndef QIUOS_DEBUG_H_
#define QIUOS_DEBUG_H_
#include "type.h"

public void debug_panic(const char *file, int line, const char *function,
                 const char *message, ...);
public void assertion_failure(char *exp, char *file, char *base_file, int line);
public void debug_backtrace(void);

#ifdef DEBUG
#define assert(exp) \
    if (exp)        \
        ;           \
    else            \
        assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#define panic(...) debug_panic(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define NOT_REACHED panic("executed an unreachable statement")
#else
#define assert(exp)
#define panic(...)
#define NOT_REACHED
#endif

#endif // QIUOS_DEBUG_H_
