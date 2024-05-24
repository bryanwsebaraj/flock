// On-disk file system format.

// Block 0 is unused.
// Block 1 is super block.
// Blocks 2 through sb.ninodes/IPB hold inodes.
// Then free bitmap blocks holding sb.size bits.
// Then sb.nblocks data blocks.
// Then sb.nlog log blocks.

#ifndef _KERN_FS_FILE_H_
#define _KERN_FS_FILE_H_

#ifdef _KERN_

#include "stat.h"
#include "inode.h"

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;  // reference count
    int8_t readable;
    int8_t writable;
    struct inode *ip;
    uint32_t off;
    bool holding_flock;
};

void file_init(void);

// Lock the file.
int file_flock(struct file *f, int operation);

// Allocate a file structure.
struct file *file_alloc(void);

// Increment ref count for file f.
struct file *file_dup(struct file *f);

// Close file f. Decrement ref count, close when reaches 0.
void file_close(struct file *f);

// Get metadata about file f.
int file_stat(struct file *f, struct file_stat *st);

// Read from file f.
int file_read(struct file *f, char *addr, int n);

// Write to file f.
int file_write(struct file *f, char *addr, int n);

#define CONSOLE 1

#endif  /* _KERN_ */

#endif  /* !_KERN_FS_FILE_H_ */
