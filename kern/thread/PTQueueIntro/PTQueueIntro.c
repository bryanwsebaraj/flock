#include <lib/x86.h>
#include <pcpu/PCPUIntro/export.h>

/**
 * The structure for thread queues.
 * The queue structure only needs to record
 * the head and tail index, since we've already implemented
 * the doubly linked list in the TCB structure.
 * This implementation is valid if at any given time, a thread
 * is in at most one thread queue.
 */
struct TQueue {
    unsigned int head;
    unsigned int tail;
};

/**
 * The mCertiKOS kernel needs NUM_IDS + NUM_CPUS thread queues.
 * The first NUM_IDS thread queues are thread sleep queues for the NUM_IDS threads/processes.
 * A thread can sleep on other thread's sleeping queue, waiting for the other thread
 * to perform some related tasks and wake it up.
 * You may not need these sleeping queues in this lab, but they will be particularly helpful
 * when you implement the inter-process communication protocols later.
 * Queues with id (NUM_IDS + cpu_id) are called ready queues, where 0 <= cpu_id < NUM_CPUS.
 * Any threads that are ready to be scheduled are pushed to a ready queue,
 * and are scheduled in a round-robin manner.
 * Note that ready queue is per-CPU data structure, thus the kernel allocates
 * one ready queue for each of its CPU.
 */
struct TQueue TQueuePool[NUM_IDS + NUM_CPUS];

unsigned int tqueue_get_head(unsigned int chid)
{
    return TQueuePool[chid].head;
}

void tqueue_set_head(unsigned int chid, unsigned int head)
{
    TQueuePool[chid].head = head;
}

unsigned int tqueue_get_tail(unsigned int chid)
{
    return TQueuePool[chid].tail;
}

void tqueue_set_tail(unsigned int chid, unsigned int tail)
{
    TQueuePool[chid].tail = tail;
}

void tqueue_init_at_id(unsigned int chid)
{
    TQueuePool[chid].head = NUM_IDS;
    TQueuePool[chid].tail = NUM_IDS;
}
