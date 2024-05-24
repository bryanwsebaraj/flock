#ifndef _USER_SPINLOCK_H_
#define _USER_SPINLOCK_H_

#include <types.h>

typedef volatile uint32_t spinlock_t;

void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);
bool spinlock_holding(spinlock_t *lk);

#endif  /* !_USER_SPINLOCK_H_ */
