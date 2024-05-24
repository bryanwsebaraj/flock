#ifndef _KERN_CV_H_
#define _KERN_CV_H_

#ifdef _KERN_

unsigned int get_curid(void);

/* thread scheduling API */
void thread_suspend(spinlock_t *lock);
void thread_wake(unsigned int id);

/* spin lock API*/
void spinlock_init(spinlock_t *lk);
void spinlock_acquire(spinlock_t *lk);
void spinlock_release(spinlock_t *lk);

void intr_local_enable(void);
void intr_local_disable(void);

#endif /* _KERN_ */

#endif /* !_KERN_CV_H_ */