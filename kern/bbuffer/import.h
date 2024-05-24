#ifndef _KERN_BBUFFER_BOUNDED_BUFFER_H
#define _KERN_BBUFFER_BOUNDED_BUFFER_H

#ifdef _KERN_

// #include <cv/export.h>
// #include <lib/spinlock.h>

/* condition variable API */
void cv_init(cv_t *cv); 
void cv_wait(cv_t *cv, spinlock_t *lock);
void cv_signal(cv_t *cv);
void cv_broadcast(cv_t *cv);

/* spin lock API*/
void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);

void intr_local_enable(void);
void intr_local_disable(void);
int get_pcpu_idx(void);
unsigned int get_curid(void);

#endif /* _KERN_ */

#endif /* _KERN_BBUFFER_BOUNDED_BUFFER_H */