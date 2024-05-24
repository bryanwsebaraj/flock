#include <lib/spinlock.h>
#include <dev/intr.h>
#include <lib/debug.h>
#include <thread/PThread/export.h>
#include <thread/PCurID/export.h>

#include "import.h"
#include "cv.h"

/* Get the size of the cv waiting threads queue */
static int cv_get_queue_size(cv_t *cv) {
	return cv->size; 
}

/* insert a thread id into the waiting queue */
static void cv_insert_into_queue(cv_t *cv, int id) {
	cv->waiting_threads[cv->write_idx] = id;
	cv->write_idx++;
	cv->write_idx %= NUM_IDS; 
	cv->size++;
}

/* remove and return an item from waiting queue */
static int cv_remove_from_queue(cv_t *cv) {
	int removed_id = cv->waiting_threads[cv->read_idx];
	cv->read_idx++;
	cv->read_idx %= NUM_IDS;
	cv->size--;
	return removed_id; 
}

/**
 * Initialize a condition variable
*/
void cv_init(cv_t *cv)
{
	cv->read_idx = 0; 
	cv->write_idx = 0;
	cv->size = 0; 
}

/**
 * Atomically release the lock and go to sleep. 
 * When thread wakes up again (on receveving a signal)
 * reacquire the lock before returning
*/
void cv_wait(cv_t *cv, spinlock_t *lock)
{
    // ensure that the lock is being held
	intr_local_disable(); 
	KERN_ASSERT(spinlock_holding(lock));
	intr_local_enable(); 

    // add this thread to the waiting queue of the condition variable 
    unsigned int cur_pid = get_curid();

	if (cv_get_queue_size(cv) == NUM_IDS) {
		// waiting queue is full
		KERN_PANIC("Waiting queue of cv variable is full!\n");
		spinlock_release(lock); 
		return; 
	} else {
		cv_insert_into_queue(cv, cur_pid); 
	}
	
	intr_local_disable();
	KERN_DEBUG("Process: %d, WAIT - CALLING thread_suspend\n", get_curid());
	intr_local_enable();

	// switch to the new thread and release lock
	intr_local_disable();
	thread_suspend(lock);
	intr_local_enable(); 

	// now back in this thread, reacquire lock
	spinlock_acquire(lock);
}

/**
 * Wake up a waiter
*/
void cv_signal(cv_t *cv)
{
	if (cv_get_queue_size(cv) == 0) return; 

	int removed_id = cv_remove_from_queue(cv); 

	intr_local_disable();
	KERN_DEBUG("Process: %d, SIGNAL - CALLING thread_wake on process %d\n", get_curid(), removed_id);
	intr_local_enable();

	intr_local_disable();
	thread_wake(removed_id);
	intr_local_enable();
}

/**
 * Wake up all waiters
*/
void cv_broadcast(cv_t *cv)
{
	int removed_id; 
	while (cv_get_queue_size(cv) != 0) {

		removed_id = cv_remove_from_queue(cv); 
		
		intr_local_disable();
		KERN_DEBUG("Process: %d, BROADCAST - CALLING thread_wake on process %d\n", get_curid(), removed_id);
		intr_local_enable();

        intr_local_disable();
		thread_wake(removed_id);	
        intr_local_enable();
	}
}