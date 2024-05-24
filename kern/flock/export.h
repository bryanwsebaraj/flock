#ifndef _KERN_FLOCK_H_
#define _KERN_FLOCK_H_

#ifdef _KERN_

#include <fs/file.h>

// Initialize a flock.
void flock_init(struct flock_t *flock);

// Acquire flock on a file. 
int flock_acquire(struct flock_t *flock, int operation);

// Release flock on a file.
int flock_release(struct flock_t *flock);
#endif /* _KERN_ */

#endif /* !_KERN_FLOCK_H_ */