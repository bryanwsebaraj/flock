#ifndef _KERN_CV_STRUCT_H
#define _KERN_CV_STRUCT_H 

#include <lib/x86.h>

typedef struct cv_t {
	int read_idx; /* index of next item to read from queue */
	int write_idx; /* index of next spot to write to queue */
	int size; 

	unsigned int waiting_threads[NUM_IDS];
} cv_t;

#endif /* _KERN_CV_STRUCT_H */