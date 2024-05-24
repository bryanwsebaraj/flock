#ifndef _KERN_FLOCK_STRUCT_H_
#define _KERN_FLOCK_STRUCT_H_

#include <lib/spinlock.h>
#include <cv/cv.h>

#define LOCK_SH 0b0001
#define LOCK_EX 0b0010
#define LOCK_UN 0b0100
#define LOCK_NB 0b1000

#define EWOULDBLOCK -2 // this needs to be negative

enum flock_state {
    INACTIVE = 0,
    SHARED,
    EXCLUSIVE
};

struct flock_t {
    int num_sh_waiting;
    int num_ex_waiting;
    int num_sh_active;

    bool ex_active;
    bool sh_active;

    enum flock_state state; 
    
    spinlock_t lock;
    cv_t ex_flock;
    cv_t sh_flock;
};

#endif /* !_KERN_FLOCK_STRUCT_H_ */