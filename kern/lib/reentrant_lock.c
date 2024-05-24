#include <lib/debug.h>
#include <lib/kstack.h>

#include "reentrant_lock.h"

#define UNLOCKED    0xFFFFFFFF

void reentrantlock_init(reentrantlock *lk)
{
    lk->lock = UNLOCKED;
    lk->count = 0u;
}

bool reentrantlock_holding(reentrantlock *lk)
{
    if (lk->count > 0u)
        return TRUE;
    else
        return FALSE;
}

void reentrantlock_acquire(reentrantlock *lk)
{
    uint32_t cpuid = get_kstack_cpu_idx();
    uint32_t lv;

    do {
        lv = cmpxchg(&lk->lock, UNLOCKED, cpuid);
    } while (lv != cpuid && lv != UNLOCKED);
    lk->count++;
}

int reentrantlock_try_acquire(reentrantlock *lk)
{
    uint32_t cpuid = get_kstack_cpu_idx();
    uint32_t lv;

    lv = cmpxchg(&lk->lock, UNLOCKED, cpuid);

    if (lv == cpuid || lv == UNLOCKED) {
        lk->count++;
        return 1;
    } else
        return 0;
}

void reentrantlock_release(reentrantlock *lk)
{
    lk->count--;
    if (lk->count == 0u) {
        xchg(&lk->lock, UNLOCKED);
    }
}
