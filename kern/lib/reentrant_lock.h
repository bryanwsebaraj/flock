#ifndef _KERN_LIB_REENTRANTLOCK_H_
#define _KERN_LIB_REENTRANTLOCK_H_

#ifdef _KERN_

#include <lib/gcc.h>
#include <lib/types.h>

typedef struct {
    volatile uint32_t lock;
    volatile uint32_t count;
} reentrantlock;

void reentrantlock_init(reentrantlock *lk);
void reentrantlock_acquire(reentrantlock *lk);
void reentrantlock_release(reentrantlock *lk);
int  reentrantlock_try_acquire(reentrantlock *lk);
bool reentrantlock_holding(reentrantlock *lock);
void reentrantlock_check (void);

#endif /* _KERN_ */

#endif /* !_SYS_PREINIT_LIB_SPINLOCK_H_ */
