#ifndef _KERN_LIB_DEBUG_H_
#define _KERN_LIB_DEBUG_H_

#ifdef _KERN_

#include "stdarg.h"

#ifdef DEBUG_MSG
#define KERN_DEBUG(...)                                \
    do {                                               \
        debug_normal(__FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define KERN_WARN(...)                               \
    do {                                             \
        debug_warn(__FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define KERN_PANIC(...)                               \
    do {                                              \
        debug_panic(__FILE__, __LINE__, __VA_ARGS__); \
    } while (0)

#define KERN_ASSERT(x)                                       \
    do {                                                     \
        if (!(x))                                            \
            KERN_PANIC("Kernel assertion failed: %s\n", #x); \
    } while (0)
#else   /* !DEBUG_MSG */
#define KERN_DEBUG(...) do {} while (0)

#define KERN_WARN(...) do {} while (0)

#define KERN_PANIC(...) do {} while (0)

#define KERN_ASSERT(c) do {} while (0)
#endif  /* DEBUG_MSG */

#define KERN_INFO(fmt, ...)             \
    do {                                \
        debug_info(fmt, ##__VA_ARGS__); \
    } while (0)

void vprintfmt(void (*putch)(int, void *), void *putdat, const char *fmt,
               va_list ap);

void debug_info(const char *fmt, ...);

#ifdef DEBUG_MSG
int dprintf(const char *fmt, ...);

void debug_normal(const char *file, int line, const char *fmt, ...);
void debug_warn(const char *file, int line, const char *fmt, ...);
void debug_panic(const char *file, int line, const char *fmt, ...);

void debug_init(void);
void debug_lock(void);
void debug_unlock(void);
#else   /* DEBUG_MSG */
#define dprintf(...) do {} while (0)
#endif  /* !DEBUG_MSG */

#endif  /* _KERN_ */

#endif  /* !_KERN_LIB_DEBUG_H_ */
