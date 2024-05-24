#include <lib/debug.h>
#include <lib/seg.h>
#include <lib/kstack.h>
#include <lib/x86.h>

#include "spinlock.h"

extern volatile uint64_t tsc_per_ms;

void gcc_inline spinlock_init(spinlock_t *lk)
{
    lk->lock_holder = NUM_CPUS + 1;
    lk->lock = 0;
}

bool gcc_inline spinlock_holding(spinlock_t *lk)
{
    if (!lk->lock)
        return FALSE;

    struct kstack *kstack = (struct kstack *) ROUNDDOWN(read_esp(), KSTACK_SIZE);
    KERN_ASSERT(kstack->magic == KSTACK_MAGIC);
    return lk->lock_holder == kstack->cpu_idx;
}

#ifdef DEBUG_DEADLOCK
void spinlock_acquire_A(spinlock_t *lk)
{
    uint64_t start_tsc = rdtsc();

    while (xchg(&lk->lock, 1) != 0) {
        if (rdtsc() - start_tsc > tsc_per_ms * 3000)
            KERN_WARN("Possible deadlock 0x%08x.\n", lk);
        pause();
    }
    struct kstack *kstack = (struct kstack *) ROUNDDOWN(read_esp(), KSTACK_SIZE);
    KERN_ASSERT(kstack->magic == KSTACK_MAGIC);
    lk->lock_holder = kstack->cpu_idx;
}
#else   /* DEBUG_DEADLOCK */
void gcc_inline spinlock_acquire_A(spinlock_t *lk)
{
    while (xchg(&lk->lock, 1) != 0)
        pause();

    struct kstack *kstack = (struct kstack *) ROUNDDOWN(read_esp(), KSTACK_SIZE);
    KERN_ASSERT(kstack->magic == KSTACK_MAGIC);
    lk->lock_holder = kstack->cpu_idx;
}
#endif  /* !DEBUG_DEADLOCK */

int gcc_inline spinlock_try_acquire_A(spinlock_t *lk)
{
    uint32_t old_val = xchg(&lk->lock, 1);
    if (old_val == 0) {
        struct kstack *kstack = (struct kstack *) ROUNDDOWN(read_esp(), KSTACK_SIZE);
        KERN_ASSERT(kstack->magic == KSTACK_MAGIC);
        lk->lock_holder = kstack->cpu_idx;
    }
    return old_val;
}

void gcc_inline spinlock_release_A(spinlock_t *lk)
{
    lk->lock_holder = NUM_CPUS + 1;
    xchg(&lk->lock, 0);
}

#ifdef DEBUG_LOCKHOLDING
extern int vdprintf(const char *fmt, va_list ap);

void spinlock_acquire_(spinlock_t *lk, ...)
{
    if (spinlock_holding(lk)) {
        while (1) {
            if (rdtsc() % 3) {
                va_list ap;
                va_start(ap, lk);
                vdprintf("Tried to self-deadlock at %s:%d\n", ap);
                va_end(ap);
            }
        }
    }

    spinlock_acquire_A(lk);
}

void spinlock_release_(spinlock_t *lk, const char *file, int line)
{
    if (!spinlock_holding(lk)) {
        KERN_PANIC("Tried to release unheld lock at %s:%d\n", file, line);
    }

    spinlock_release_A(lk);
}

int spinlock_try_acquire_(spinlock_t *lk, const char *file, int line)
{
    if (spinlock_holding(lk)) {
        KERN_PANIC("Tried to self-deadlock at %s:%d\n", file, line);
    }

    return spinlock_try_acquire_A(lk);
}
#else   /* DEBUG_LOCKHOLDING */
void gcc_inline spinlock_acquire(spinlock_t *lk)
{
    spinlock_acquire_A(lk);
}

void gcc_inline spinlock_release(spinlock_t *lk)
{
    spinlock_release_A(lk);
}

int gcc_inline spinlock_try_acquire(spinlock_t *lk)
{
    return spinlock_try_acquire_A(lk);
}
#endif  /* !DEBUG_LOCKHOLDING */
