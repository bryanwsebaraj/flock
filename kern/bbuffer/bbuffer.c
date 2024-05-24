#include <cv/export.h>
#include <cv/cv.h>
#include <lib/spinlock.h>
#include <dev/intr.h>
#include <lib/debug.h>

#include "import.h"

#define CAPACITY 5

typedef struct bbuffer {
    unsigned int read_idx; 
    unsigned int write_idx;  
    spinlock_t lock; 

    cv_t bufferIsEmpty; 
    cv_t bufferIsFull; 

	int size;
	int buffer[CAPACITY];
} bbuffer; 

static bbuffer global_buffer; 

/* Get the size of global_buffer */
static int get_buffer_size() {
	return global_buffer.size; 
}

/* insert an item into global_buffer */
static void insert_into_buffer(int item) {
	global_buffer.buffer[global_buffer.write_idx] = item;
	global_buffer.write_idx++;
	global_buffer.write_idx %= CAPACITY; 
	global_buffer.size++; 
}

/* remove and return an item from global_buffer */
static int remove_from_buffer() {
	int removed_item = global_buffer.buffer[global_buffer.read_idx];
	global_buffer.read_idx++;
	global_buffer.read_idx %= CAPACITY;
	global_buffer.size--; 
	return removed_item;
}

/**
 * Initialize the bounded buffer.
 * Only called once (during boot)
*/
void bbuffer_init(void)
{
	global_buffer.read_idx = 0;
	global_buffer.write_idx = 0;
	spinlock_init(&global_buffer.lock);

	global_buffer.size = 0;
	cv_init(&global_buffer.bufferIsEmpty);
	cv_init(&global_buffer.bufferIsFull); 
}

/**
 * Adds item to the buffer; waits until the buffer is 
 * not full to add the item 
*/
void bbuffer_insert_item(int item)
{
	spinlock_acquire(&global_buffer.lock);

	while (get_buffer_size() >= CAPACITY) {
		cv_wait(&global_buffer.bufferIsFull, &global_buffer.lock);
	}

	// intr_local_disable();
	// KERN_DEBUG("CPU: %d, Process: %d, START Inserting item %d into buffer\n", get_pcpu_idx(), get_curid(), item);
	// KERN_DEBUG("global_buffer.size = %d\n", get_buffer_size());
	// intr_local_enable();

	insert_into_buffer(item);

	// intr_local_disable();
	// KERN_DEBUG("CPU: %d, Process: %d, DONE inserting item %d into buffer\n", get_pcpu_idx(), get_curid(), item);
	// KERN_DEBUG("global_buffer.size = %d\n", global_buffer.size);
	// intr_local_enable();

	cv_signal(&global_buffer.bufferIsEmpty); 
	spinlock_release(&global_buffer.lock); 
}

/**
 * Removes an item from the buffer; waits until the buffer
 * has at least one item before removing it
*/
int bbuffer_remove_item(void)
{
	int item;

	spinlock_acquire(&global_buffer.lock);
	
	while (get_buffer_size() == 0) {
		cv_wait(&global_buffer.bufferIsEmpty, &global_buffer.lock);
	}

	// intr_local_disable();
	// KERN_DEBUG("CPU: %d, Process: %d, START Removing item from buffer\n", get_pcpu_idx(), get_curid());
	// KERN_DEBUG("global_buffer.size = %d\n", get_buffer_size());
	// intr_local_enable(); 

	item = remove_from_buffer();

	// intr_local_disable();
	// KERN_DEBUG("CPU: %d, Process: %d, DONE Removing item %d from buffer\n", get_pcpu_idx(), get_curid(), item);
	// KERN_DEBUG("global_buffer.size = %d\n", get_buffer_size());
	// intr_local_enable(); 

	cv_signal(&global_buffer.bufferIsFull); 
	spinlock_release(&global_buffer.lock);
	return item; 
}