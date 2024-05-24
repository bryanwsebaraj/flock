// File descriptors

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/spinlock.h>
#include "params.h"
#include "stat.h"
#include "dinode.h"
#include "inode.h"
#include "file.h"
#include "log.h"

#include <kern/flock/flock.h>
#include <kern/flock/export.h>

struct {
    spinlock_t lock;
    struct file file[NFILE];
} ftable;

void file_init(void)
{
    spinlock_init(&ftable.lock);
}

int file_flock(struct file *f, int operation) {
    struct file_stat stat;
    int result;

    if (file_stat(f, &stat) < 0) return -1; 
    if (stat.type != T_FILE) return -1;

    if (operation & LOCK_EX) {

        if (operation & LOCK_SH || operation & LOCK_UN) return -1; 

        if (f->holding_flock) {
            f->holding_flock = FALSE;
            if (flock_release(&f->ip->flock) == -1) return -1;
        } 
        result = flock_acquire(&f->ip->flock, operation);
        if (result >= 0) f->holding_flock = TRUE;
        return result; 
        
    } else if (operation & LOCK_SH) {

        if (operation & LOCK_EX || operation & LOCK_UN) return -1;

        if (f->holding_flock) {
            f->holding_flock = FALSE;
            if (flock_release(&f->ip->flock) < 0) return -1;
        } 
        result = flock_acquire(&f->ip->flock, operation);
        if (result >= 0) f->holding_flock = TRUE;
        return result; 

    } else if (operation & LOCK_UN) {

        if (operation & LOCK_EX || operation & LOCK_SH) return -1; 

        if (f->holding_flock == FALSE) return -1;

        result = flock_release(&f->ip->flock);

        if (result < 0) return -1;

        f->holding_flock = FALSE;

        return result;

    }

    // none of the operation bits were set
    return -1;
}

/**
 * Allocate a file structure.
 */
struct file *file_alloc(void)
{
    struct file *f;

    spinlock_acquire(&ftable.lock);
    for (f = ftable.file; f < ftable.file + NFILE; f++) {
        if (f->ref == 0) {
            f->ref = 1;
            spinlock_release(&ftable.lock);
            return f;
        }
    }
    spinlock_release(&ftable.lock);
    return 0;
}

/**
 * Increment ref count for file f.
 */
struct file *file_dup(struct file *f)
{
    spinlock_acquire(&ftable.lock);
    if (f->ref < 1)
        KERN_PANIC("file_dup");
    f->ref++;
    spinlock_release(&ftable.lock);
    return f;
}

/**
 * Close file f. Decrement ref count, close when reaches 0.
 */
void file_close(struct file *f)
{
    struct file ff;

    spinlock_acquire(&ftable.lock);
    if (f->ref < 1)
        KERN_PANIC("file_close");
    if (f->holding_flock) file_flock(f, LOCK_UN);
    if (--f->ref > 0) {
        spinlock_release(&ftable.lock);
        return;
    }
    ff = *f;
    f->ref = 0;
    f->type = FD_NONE;
    spinlock_release(&ftable.lock);

    if (ff.type == FD_INODE) {
        begin_trans();
        inode_put(ff.ip);
        commit_trans();
    }
}

/**
 * Get metadata about file f.
 */
int file_stat(struct file *f, struct file_stat *st)
{
    if (f->type == FD_INODE) {
        inode_lock(f->ip);
        inode_stat(f->ip, st);
        inode_unlock(f->ip);
        return 0;
    }
    return -1;
}

/**
 * Read from file f.
 */
int file_read(struct file *f, char *addr, int n)
{
    int r;

    if (f->readable == 0)
        return -1;
    if (f->type == FD_INODE) {
        inode_lock(f->ip);
        if ((r = inode_read(f->ip, addr, f->off, n)) > 0)
            f->off += r;
        inode_unlock(f->ip);
        return r;
    }
    KERN_PANIC("file_read");
    return -1;
}

/**
 * Write to file f.
 */
int file_write(struct file *f, char *addr, int n)
{
    int r;

    if (f->writable == 0)
        return -1;
    if (f->type == FD_INODE) {
        // Write a few blocks at a time to avoid exceeding
        // the maximum log transaction size, including
        // i-node, indirect block, allocation blocks,
        // and 2 blocks of slop for non-aligned writes.
        // this really belongs lower down, since inode_write()
        // might be writing a device like the console.
        int max = ((LOGSIZE - 1 - 1 - 2) / 2) * 512;
        int i = 0;
        while (i < n) {
            int n1 = n - i;
            if (n1 > max)
                n1 = max;

            begin_trans();
            inode_lock(f->ip);
            if ((r = inode_write(f->ip, addr + i, f->off, n1)) > 0)
                f->off += r;
            inode_unlock(f->ip);
            commit_trans();

            if (r < 0)
                break;
            if (r != n1)
                KERN_PANIC("short file_write");
            i += r;
        }
        return i == n ? n : -1;
    }
    KERN_PANIC("file_write");
    return -1;
}

