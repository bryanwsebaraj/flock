#ifndef _KERN_FLOCK_H_
#define _KERN_FLOCK_H_

#ifdef _KERN_

/* spin lock API*/
void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);

/* condition variable API */
void cv_init(cv_t *cv); 
void cv_wait(cv_t *cv, spinlock_t *lock);
void cv_signal(cv_t *cv);
void cv_broadcast(cv_t *cv); 

#endif /* _KERN_ */

#endif /* !_KERN_FLOCK_H_ */