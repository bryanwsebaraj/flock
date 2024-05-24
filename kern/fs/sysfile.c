// File-system system calls.

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/pmap.h>
#include <kern/lib/string.h>
#include <kern/lib/trap.h>
#include <kern/lib/spinlock.h>
#include <kern/lib/syscall.h>
#include <kern/thread/PTCBIntro/export.h>
#include <kern/thread/PCurID/export.h>
#include <kern/trap/TSyscallArg/export.h>

#include "dir.h"
#include "path.h"
#include "file.h"
#include "fcntl.h"
#include "log.h"

#define BUFLEN_MAX 10000
char fsbuf[BUFLEN_MAX];
spinlock_t fsbuf_lk;

static char *flock_operations[4] = {
    "LOCK_SH",
    "LOCK_EX",
    "LOCK_UN",
    "LOCK_NB"
};

void fs_init(void) {
    spinlock_init(&fsbuf_lk);
}

static bool check_buf(tf_t *tf, uintptr_t buf, size_t len, size_t maxlen) {
    if (!(VM_USERLO <= buf && buf + len <= VM_USERHI)
        || (0 < maxlen && maxlen <= len)) {
        syscall_set_errno(tf, E_INVAL_ADDR);
        syscall_set_retval1(tf, -1);
        return FALSE;
    }
    return TRUE;
}

/**
 * This function is not a system call handler, but an auxiliary function
 * used by sys_open.
 * Allocate a file descriptor for the given file.
 * You should scan the list of open files for the current thread
 * and find the first file descriptor that is available.
 * Return the found descriptor or -1 if none of them is free.
 */
static int fdalloc(struct file *f)
{
    int fd;
    int pid = get_curid();
    struct file **openfiles = tcb_get_openfiles(pid);

    for (fd = 0; fd < NOFILE; fd++) {
        if (openfiles[fd] == NULL) {
            tcb_set_openfiles(pid, fd, f);
            return fd;
        }
    }

    return -1;
}

/**
 * From the file indexed by the given file descriptor, read n bytes and save them
 * into the buffer in the user. As explained in the assignment specification,
 * you should first write to a kernel buffer then copy the data into user buffer
 * with pt_copyout.
 * Return Value: Upon successful completion, read() shall return a non-negative
 * integer indicating the number of bytes actually read. Otherwise, the
 * functions shall return -1 and set errno E_BADF to indicate the error.
 */
void sys_read(tf_t *tf)
{
    struct file *file;
    int read;

    int pid = get_curid();
    int fd = syscall_get_arg2(tf);
    uintptr_t buf = syscall_get_arg3(tf);
    size_t buflen = syscall_get_arg4(tf);

    if (!check_buf(tf, buf, buflen, BUFLEN_MAX)) {
        return;
    }
    if (!(0 <= fd && fd < NOFILE)) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    file = tcb_get_openfiles(pid)[fd];
    if (file == NULL || file->type != FD_INODE) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    spinlock_acquire(&fsbuf_lk);
    read = file_read(file, fsbuf, buflen);
    syscall_set_retval1(tf, read);
    if (0 <= read) {
        pt_copyout(fsbuf, pid, buf, buflen);
        syscall_set_errno(tf, E_SUCC);
    }
    else {
        syscall_set_errno(tf, E_BADF);
    }
    spinlock_release(&fsbuf_lk);
}

/**
 * Write n bytes of data in the user's buffer into the file indexed by the file descriptor.
 * You should first copy the data info an in-kernel buffer with pt_copyin and then
 * pass this buffer to appropriate file manipulation function.
 * Upon successful completion, write() shall return the number of bytes actually
 * written to the file associated with f. This number shall never be greater
 * than nbyte. Otherwise, -1 shall be returned and errno E_BADF set to indicate the
 * error.
 */
void sys_write(tf_t *tf)
{
    struct file *file;
    int written;

    int pid = get_curid();
    int fd = syscall_get_arg2(tf);
    uintptr_t buf = syscall_get_arg3(tf);
    size_t buflen = syscall_get_arg4(tf);

    if (!check_buf(tf, buf, buflen, BUFLEN_MAX)) {
        return;
    }
    if (!(0 <= fd && fd < NOFILE)) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    file = tcb_get_openfiles(pid)[fd];
    if (file == NULL || file->type != FD_INODE) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    spinlock_acquire(&fsbuf_lk);
    pt_copyin(pid, buf, fsbuf, buflen);
    written = file_write(file, fsbuf, buflen);
    spinlock_release(&fsbuf_lk);

    syscall_set_retval1(tf, written);
    if (0 <= written) {
        syscall_set_errno(tf, E_SUCC);
    }
    else {
        syscall_set_errno(tf, E_BADF);
    }
}

/**
 * Return Value: Upon successful completion, 0 shall be returned; otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_close(tf_t *tf)
{
    struct file *file;

    int pid = get_curid();
    int fd = syscall_get_arg2(tf);

    if (!(0 <= fd && fd < NOFILE)) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    file = tcb_get_openfiles(pid)[fd];
    if (file == NULL || file->ref < 1) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    file_close(file);
    tcb_set_openfiles(pid, fd, NULL);

    syscall_set_errno(tf, E_SUCC);
    syscall_set_retval1(tf, 0);
}

/**
 * Return Value: Upon successful completion, 0 shall be returned. Otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_fstat(tf_t *tf)
{
    struct file *file;
    struct file_stat fs_stat;

    int pid = get_curid();
    int fd = syscall_get_arg2(tf);
    uintptr_t stat = syscall_get_arg3(tf);

    if (!check_buf(tf, stat, sizeof(struct file_stat), 0)) {
        return;
    }
    if (!(0 <= fd && fd < NOFILE)) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    file = tcb_get_openfiles(pid)[fd];
    if (file == NULL || file->type != FD_INODE) {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
        return;
    }

    if (file_stat(file, &fs_stat) == 0) {
        pt_copyout(&fs_stat, pid, stat, sizeof(struct file_stat));
        syscall_set_errno(tf, E_SUCC);
        syscall_set_retval1(tf, 0);
    }
    else {
        syscall_set_errno(tf, E_BADF);
        syscall_set_retval1(tf, -1);
    }
}

/**
 * Create the path new as a link to the same inode as old.
 */
void sys_link(tf_t * tf)
{
    char name[DIRSIZ], new[128], old[128];
    struct inode *dp, *ip;

    uintptr_t uold = syscall_get_arg2(tf);
    uintptr_t unew = syscall_get_arg3(tf);
    uintptr_t oldlen = syscall_get_arg4(tf);
    uintptr_t newlen = syscall_get_arg5(tf);

    if (!check_buf(tf, uold, oldlen, 128) || !check_buf(tf, unew, newlen, 128)) {
        return;
    }

    pt_copyin(get_curid(), uold, old, oldlen);
    pt_copyin(get_curid(), unew, new, newlen);

    if ((ip = namei(old)) == 0) {
        syscall_set_errno(tf, E_NEXIST);
        return;
    }

    begin_trans();

    inode_lock(ip);
    if (ip->type == T_DIR) {
        inode_unlockput(ip);
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    ip->nlink++;
    inode_update(ip);
    inode_unlock(ip);

    if ((dp = nameiparent(new, name)) == 0)
        goto bad;
    inode_lock(dp);
    if (dp->dev != ip->dev || dir_link(dp, name, ip->inum) < 0) {
        inode_unlockput(dp);
        goto bad;
    }
    inode_unlockput(dp);
    inode_put(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_lock(ip);
    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

/**
 * Is the directory dp empty except for "." and ".." ?
 */
static int isdirempty(struct inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (inode_read(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
            KERN_PANIC("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

void sys_unlink(tf_t *tf)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ], path[128];
    uint32_t off;

    uintptr_t buf = syscall_get_arg2(tf);
    size_t buflen = syscall_get_arg3(tf);

    if (!check_buf(tf, buf, buflen, 128)) {
        return;
    }

    pt_copyin(get_curid(), buf, path, buflen);

    if ((dp = nameiparent(path, name)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    begin_trans();

    inode_lock(dp);

    // Cannot unlink "." or "..".
    if (dir_namecmp(name, ".") == 0 || dir_namecmp(name, "..") == 0)
        goto bad;

    if ((ip = dir_lookup(dp, name, &off)) == 0)
        goto bad;
    inode_lock(ip);

    if (ip->nlink < 1)
        KERN_PANIC("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip)) {
        inode_unlockput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));
    if (inode_write(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
        KERN_PANIC("unlink: writei");
    if (ip->type == T_DIR) {
        dp->nlink--;
        inode_update(dp);
    }
    inode_unlockput(dp);

    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_unlockput(dp);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

static struct inode *create(char *path, short type, short major, short minor)
{
    uint32_t off;
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = nameiparent(path, name)) == 0)
        return 0;
    inode_lock(dp);

    if ((ip = dir_lookup(dp, name, &off)) != 0) {
        inode_unlockput(dp);
        inode_lock(ip);
        if (type == T_FILE && ip->type == T_FILE)
            return ip;
        inode_unlockput(ip);
        return 0;
    }

    if ((ip = inode_alloc(dp->dev, type)) == 0)
        KERN_PANIC("create: ialloc");

    inode_lock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    inode_update(ip);

    if (type == T_DIR) {  // Create . and .. entries.
        dp->nlink++;      // for ".."
        inode_update(dp);
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (dir_link(ip, ".", ip->inum) < 0
            || dir_link(ip, "..", dp->inum) < 0)
            KERN_PANIC("create dots");
    }

    if (dir_link(dp, name, ip->inum) < 0)
        KERN_PANIC("create: dir_link");

    inode_unlockput(dp);
    return ip;
}

void sys_open(tf_t *tf)
{
    char path[128];
    int fd, omode;
    struct file *f;
    struct inode *ip;

    uintptr_t buf = syscall_get_arg2(tf);
    size_t buflen = syscall_get_arg4(tf);

    // KERN_DEBUG("Process: %d, beginning sys_open() on %p\n", get_curid(), buf);

    if (!check_buf(tf, buf, buflen, 128)) {
        // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
        return;
    }

    pt_copyin(get_curid(), buf, path, buflen);
    omode = syscall_get_arg3(tf);

    if (omode & O_CREATE) {
        // KERN_DEBUG("Process: %d, creating flockfile\n", get_curid());
        begin_trans();
        ip = create(path, T_FILE, 0, 0);
        commit_trans();
        if (ip == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_CREATE);
            // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
            return;
        }
    } else {
        // KERN_DEBUG("Process: %d, START check inode\n");
        if ((ip = namei(path)) == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_NEXIST);
            // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
            return;
        }
        inode_lock(ip);
        if (ip->type == T_DIR && omode != O_RDONLY) {
            inode_unlockput(ip);
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_DISK_OP);
            // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
            return;
        }
        // KERN_DEBUG("Process: %d, END check inode\n");
    }

    // KERN_DEBUG("Process: %d, START allocating file structure\n");
    if ((f = file_alloc()) == 0 || (fd = fdalloc(f)) < 0) {
        if (f)
            file_close(f);
        inode_unlockput(ip);
        syscall_set_retval1(tf, -1);
        syscall_set_errno(tf, E_DISK_OP);
        // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
        return;
    }
    inode_unlock(ip);
    // KERN_DEBUG("Process: %d, END allocating file structure\n", get_curid());

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
    syscall_set_retval1(tf, fd);
    syscall_set_errno(tf, E_SUCC);
    // KERN_DEBUG("Process: %d, ending sys_open()\n", get_curid());
}

void sys_mkdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;

    uintptr_t buf = syscall_get_arg2(tf);
    size_t buflen = syscall_get_arg3(tf);

    if (!check_buf(tf, buf, buflen, 128)) {
        return;
    }

    pt_copyin(get_curid(), buf, path, buflen);

    begin_trans();
    if ((ip = (struct inode *) create(path, T_DIR, 0, 0)) == 0) {
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_SUCC);
}

void sys_chdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;
    int pid = get_curid();

    uintptr_t buf = syscall_get_arg2(tf);
    size_t buflen = syscall_get_arg3(tf);

    if (!check_buf(tf, buf, buflen, 128)) {
        return;
    }

    pt_copyin(get_curid(), buf, path, buflen);

    if ((ip = namei(path)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_lock(ip);
    if (ip->type != T_DIR) {
        inode_unlockput(ip);
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlock(ip);
    inode_put(tcb_get_cwd(pid));
    tcb_set_cwd(pid, ip);
    syscall_set_errno(tf, E_SUCC);
}

void sys_flock(tf_t *tf) {
    int fd = syscall_get_arg2(tf), op = syscall_get_arg3(tf);
    KERN_DEBUG("sys_flock fd %d. operation %s\n", fd, flock_operations[op / 2]);
    if (fd < 0 || fd >= NOFILE) {
        syscall_set_retval1(tf, -1);
        syscall_set_errno(tf, E_BADF);
        return;
    }
    struct file *f = tcb_get_openfiles(get_curid())[fd];
    if (f != NULL && f->type == FD_INODE) {
        if (file_flock(f, op) >= 0) {
            syscall_set_retval1(tf, 0);
            syscall_set_errno(tf, E_SUCC);
        } else {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_BADF);
        }
    } else {
        syscall_set_retval1(tf, -1);
        syscall_set_errno(tf, E_BADF);
        return;
    }
}