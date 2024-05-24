#ifndef _KERN_CV_H_
#define _KERN_CV_H_

#ifdef _KERN_

#include "cv.h"
#include <lib/spinlock.h>

void cv_init(cv_t *cv); 
void cv_wait(cv_t *cv, spinlock_t *lock);
void cv_signal(cv_t *cv);
void cv_broadcast(cv_t *cv); 

#endif /* _KERN_ */

#endif /* _KERN_CV_H_ */