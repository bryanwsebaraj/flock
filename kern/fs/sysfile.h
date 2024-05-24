// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into inode.c, dir.c, path.c and file.c.

#ifndef _KERN_FS_SYSFILE_H_
#define _KERN_FS_SYSFILE_H_

#ifdef _KERN_

void sys_read(tf_t *tf);
void sys_write(tf_t *tf);
void sys_close(tf_t *tf);
void sys_fstat(tf_t *tf);
void sys_link(tf_t *tf);
void sys_unlink(tf_t *tf);
void sys_open(tf_t *tf);
void sys_mkdir(tf_t *tf);
void sys_chdir(tf_t *tf);
void sys_flock(tf_t *tf);

#endif  /* _KERN_ */

#endif  /* !_KERN_FS_SYSFILE_H_ */
