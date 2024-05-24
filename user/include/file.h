#ifndef _USER_FILE_H_
#define _USER_FILE_H_

#define DIRSIZ 14

struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

#define T_DIR  1  // Directory
#define T_FILE 2  // File
#define T_DEV  3  // Device

struct file_stat {
    int16_t type;    // Type of file
    uint32_t dev;    // File system's disk device
    uint32_t ino;    // Inode number
    uint16_t nlink;  // Number of links to file
    size_t size;     // Size of file in bytes
};

#define BSIZE     512            // block size
#define NDIRECT   12
#define NINDIRECT (BSIZE / sizeof(uint32_t))
#define MAXFILE   (NDIRECT + NINDIRECT)

struct inode {
    uint32_t dev;   // Device number
    uint32_t inum;  // Inode number
    int ref;        // Reference count
    int32_t flags;  // I_BUSY, I_VALID

    int16_t type;   // Copy of disk inode
    int16_t major;
    int16_t minor;
    int16_t nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;  // Reference count
    int8_t readable;
    int8_t writable;
    struct inode *ip;
    uint32_t off;
};

// fcntl.h
#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR   0x002
#define O_CREATE 0x200

// /kern/flock/flock.h
#define LOCK_SH 0b0001 
#define LOCK_EX 0b0010
#define LOCK_UN 0b0100
#define LOCK_NB 0b1000
#define EWOULDBLOCK -2 

#include <syscall.h>

#define read(fd, str, n)  sys_read((fd), (str), (n))
#define write(fd, str, n) sys_write((fd), (str), (n))
#define close(fd)         sys_close((fd))
#define fstat(fd, stat)   sys_fstat((fd), (stat))
#define link(old, new)    sys_link((old), (new))
#define unlink(path)      sys_unlink((path))
#define open(path, omode) sys_open((path), (omode))
#define mkdir(path)       sys_mkdir((path))
#define chdir(path)       sys_chdir((path))
#define flock(fd, op)       sys_flock((fd), (op))

#endif  /* !_USER_FILE_H_ */
