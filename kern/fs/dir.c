#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include "inode.h"
#include "dir.h"

// Directories

int dir_namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

/**
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode *dir_lookup(struct inode *dp, char *name, uint32_t * poff)
{
    uint32_t off;
    struct dirent de;

    if (dp->type != T_DIR)
        KERN_PANIC("dir_lookup not DIR");

    for (off = 0; off < dp->size; off += sizeof(struct dirent)) {
        KERN_ASSERT(inode_read(dp, (char *) &de, off, sizeof(struct dirent)) == sizeof(struct dirent));
        if (dir_namecmp(de.name, name) == 0 && de.inum != 0) {
            if (poff != NULL) {
                *poff = off;
            }
            return inode_get(dp->dev, de.inum);
        }
    }

    return NULL;
}

// Write a new directory entry (name, inum) into the directory dp.
int dir_link(struct inode *dp, char *name, uint32_t inum)
{
    uint32_t off;
    struct inode *tmp;
    struct dirent de;

    // Check that name is not present.
    if ((tmp = dir_lookup(dp, name, &off)) != NULL) {
        inode_put(tmp);
        return -1;
    }

    // Look for an empty dirent.
    for (off = 0; off < dp->size; off += sizeof(struct dirent)) {
        KERN_ASSERT(inode_read(dp, (char *) &de, off, sizeof(struct dirent)) == sizeof(struct dirent));
        if (de.inum == 0) {
            break;
        }
    }

    de.inum = inum;
    strncpy(de.name, name, DIRSIZ);
    KERN_ASSERT(inode_write(dp, (char *) &de, off, sizeof(struct dirent)) == sizeof(struct dirent));
    return 0;
}
