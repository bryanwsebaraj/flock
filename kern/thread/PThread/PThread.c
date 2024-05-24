#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>
#include <kern/thread/PTCBIntro/export.h>

#include "import.h"

static spinlock_t sched_lk;

unsigned int sched_ticks[NUM_CPUS];

void thread_init(unsigned int mbi_addr)
{
    unsigned int i;
    for (i = 0; i < NUM_CPUS; i++) {
        sched_ticks[i] = 0;
    }

    spinlock_init(&sched_lk);
    tqueue_init(mbi_addr);
    set_curid(0);
    tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates a new child thread context, sets the state of the new child thread
 * to ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
    unsigned int pid;

    spinlock_acquire(&sched_lk);

    pid = kctx_new(entry, id, quota);
    if (pid != NUM_IDS) {
        tcb_set_state(pid, TSTATE_READY);
        tqueue_enqueue(NUM_IDS, pid);
    }

    spinlock_release(&sched_lk);

    return pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * Then set the state of the popped thread as running, set the
 * current thread id, and switch to the new kernel context.
 * Hint: If you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    spinlock_acquire(&sched_lk);

    old_cur_pid = get_curid();
    tcb_set_state(old_cur_pid, TSTATE_READY);
    tqueue_enqueue(NUM_IDS, old_cur_pid);

    new_cur_pid = tqueue_dequeue(NUM_IDS);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    if (old_cur_pid != new_cur_pid) {
        spinlock_release(&sched_lk);
        kctx_switch(old_cur_pid, new_cur_pid);
    }
    else {
        spinlock_release(&sched_lk);
    }
}

void sched_update(void)
{
    spinlock_acquire(&sched_lk);
    sched_ticks[get_pcpu_idx()] += 1000 / LAPIC_TIMER_INTR_FREQ;
    if (sched_ticks[get_pcpu_idx()] >= SCHED_SLICE) {
        sched_ticks[get_pcpu_idx()] = 0;
        spinlock_release(&sched_lk);
        thread_yield();
    }
    else {
        spinlock_release(&sched_lk);
    }
}

/**
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void thread_sleep(void *chan, spinlock_t *lk)
{
	// KERN_DEBUG("Process: %d, CALLING thread_sleep on chan %p\n", get_curid(), chan);
    // TODO: your local variables here.
    unsigned int old_cur_pid;
    unsigned int new_cur_pid;

    if (lk == 0)
        KERN_PANIC("sleep without lock");

    // Must acquire sched_lk in order to change the current thread's state and
    // then switch. Once we hold sched_lk, we can be guaranteed that we won't
    // miss any wakeup (wakeup runs with sched_lk locked), so it's okay to
    // release lock.
    spinlock_acquire(&sched_lk);
    spinlock_release(lk);

    // Go to sleep.
    old_cur_pid = get_curid();
    new_cur_pid = tqueue_dequeue(NUM_IDS);
    KERN_ASSERT(new_cur_pid != NUM_IDS);
    tcb_set_chan(old_cur_pid, chan);
    tcb_set_state(old_cur_pid, TSTATE_SLEEP);
    tcb_set_state(new_cur_pid, TSTATE_RUN);
    set_curid(new_cur_pid);

    // Context switch.
    spinlock_release(&sched_lk);
    kctx_switch(old_cur_pid, new_cur_pid);
    spinlock_acquire(&sched_lk);

    // Tidy up.
    tcb_set_chan(old_cur_pid, NULL);

    // Reacquire original lock.
    spinlock_acquire(lk);
    spinlock_release(&sched_lk);
}

/**
 * Wake up all processes sleeping on chan.
 */
void thread_wakeup(void *chan)
{
    unsigned int pid;
    spinlock_acquire(&sched_lk);

    for (pid = 0; pid < NUM_IDS; pid++) {
        if (chan == tcb_get_chan(pid)) {
            // KERN_DEBUG("Process: %d, CALLING thread_wakeup on process %d from chan %p\n", get_curid(), pid, chan);
            tcb_set_state(pid, TSTATE_READY);
            tqueue_enqueue(NUM_IDS, pid);
        }
    }

    spinlock_release(&sched_lk);
}


/**
 * Suspend the running of the current thread, switch to a new thread.
 * Note: the thread being suspended is assumed to be stored in a "sleeping queue" elsewhere
 * and thus is not stored in any queue explicitly within this implementation
*/
void thread_suspend(spinlock_t *lock)
{
	// implementation follows Scheduler::suspend() in OS book
	spinlock_acquire(&sched_lk);
	spinlock_release(lock);
	
	unsigned int cur_pid = get_curid(); 
	tcb_set_state(cur_pid, TSTATE_SLEEP); // maybe add a new state called TSTATE_WAITING?

	unsigned int new_pid = tqueue_dequeue(NUM_IDS); // removed + get_pcpu_idx()
    KERN_ASSERT(new_pid != NUM_IDS);
	tcb_set_state(new_pid, TSTATE_RUN); 
	set_curid(new_pid);

    // TODO: acquire lock before releasing sched_lk?

	if (cur_pid != new_pid) {
		spinlock_release(&sched_lk);
		kctx_switch(cur_pid, new_pid); 
		// note: the book implementation has the lock being released after the thread has switched
	} else {
		spinlock_release(&sched_lk);
	}
}

/**
 * Place the thread in the ready queue for its own CPU.
 * Note: this implementation might be risky? 
 * (threads on different CPUs can queue on each others' ready queues)
*/
void thread_wake(unsigned int id)
{
	// implementation follows Scheduler::makeReady() in OS book 
	unsigned int cpu_num = tcb_get_cpu(id);
	spinlock_acquire(&sched_lk);
	tcb_set_state(id, TSTATE_READY);
	tqueue_enqueue(NUM_IDS, id); // removed NUM_IDS + cpu_num 
	spinlock_release(&sched_lk); 
}