# Flock

I implemented the flock syscall based on the Linux version (https://man7.org/linux/man-pages/man2/flock.2.html), which provides an advisory file lock on any open file, for simplified mCertiKOS, as my final project for CPSC 422 - Operating Systems. 

## Features

Participation in the file lock is voluntary; if a thread elects to participate, it does so by calling flock (fd, LOCK_FN), where LOCK_FN can be LOCK_SH, LOCK_EX, or LOCK_UN, to either request to acquire a shared file lock, acquire an exclusive file lock, or release the file lock, respectively. Requests are granted asynchronously unless otherwise specified, where if a flock cannot be granted immediately, the thread scheduler (via a monitor with two condition variables) puts the thread to sleep, allowing the thread with the file lock to progress until the file lock (voluntarily) becomes available again. However, if a file lock request is made in non-blocking mode (LOCK_NB), it is treated as a request for an immediate atomic file lock acquisition, where the file lock is granted if and only if it is immediately accessible or refused if it is not, without putting the user thread to sleep.

In my implementation, the inode pointed to by the file contains the flock data and monitor for the thread scheduler, but each file pointer contains the information for the thread's flock access to the inode. A file cannot simulataneously have both shared and exclusive locks on the inode. 

If a 'file' is locked under an exclusive lock, all attempts to access the inode which is pointed to by the file (as different threads may have different file descriptors) will wait until completion, where the threads do not progress until the lock is released. If a 'file' is locked under a shared lock, all valid attempts to access the flock will depend on the lock request. If it is an exclusive request, the request will be queued and if it is a shared request (non-blocking or blocking), it will be granted immediately (with one exception, read below). 

My flock implementation implements an exclusive-preferred shared-exclusive lock, similar to the common write-preferring RW locks (note that all writes should only be done under an exclusive flock anyways). This avoids the problem of exclusive starvation by preventing any new shared flock requests from threads (which must be readers) from acquiring the lock if there is an exclusive flock request queued and waiting for the lock; the thread that made the exclusive flock request will acquire the lock as soon as all threads which were holding the shared flock have completed. This means that all exclusive file lock requests are granted before any new shared file lock request, regardless of the order in which threads make them.

Identical to the linux implementation of flock, flock conversion from exclusive to shared and vice versa relies on a non-atomic handler (i.e. the existing lock may be removed by one thread and then acquired by another thread before it can be switched for the original thread), but the process of granting the flock (by manipulating the underlying inode metadata and waking up/sleeping threds) is atomic. 

## Relevant Code

While many parts of this codebase/kernel were written throughout this course, the most relevant code for this project can be found in the following directories/files:
 - Flock code: './kern/flock', './kern/fs/file.c'
 - Syscall overhead: './kern/fs/sysfile.c', './kern/trap/TSyscall/TSyscall.c', './user/include/syscall.h' 
 - Condition variable code for use in monitor: './kern/cv'
 - Thread scheduling: './kern/thread/Pthread/Pthread.c'
 - Tests: './user/flocktest'

## Testing

The tests are organized in the following manner:

- flocktest.c (in conjunction with flockstall.c) tests the basic functionality of flock. This includes testing the following:
    - A user process trying to obtain a shared file lock 
    - A user process trying to obtain an exclusive file lock 
    - A lock on a file descriptor is automatically released when close() is called on that file descriptor 
    - A user process trying to use the non-blocking flag (LOCK_NB) when obtaining a lock
    - A user process trying to upgrade a lock (change the lock held from shared to exclusive) non-atomically
    - A user process trying to downgrade a lock (change the lock held from exclusive to shared) non-atomically

- flockdemo.c displays the functionality of the lock with multiple processes that hold shared/exclusive locks on a single file. It works in conjunction with flockreader.c (a process that obtains a shared lock and reads from a file) and flockwriter.c (a process that obtains an exclusive lock and writes to a file). 

To run the tests:

- Install qemu: https://www.qemu.org/download/
- Then, at the root of this directory:
    - Compile: 'make'
    - Run: 'make qemu-nox'
        - To terminate qemu: 'Ctrl-A', followed by 'x'
    - Clean: 'make clean'

Note: This codebase was tested on frog.zoo.cs.yale.edu, a remote Linux (Fedora) computer on Yale's internal network. 

I would like to give credit to the teaching staff of the course for such a fantastic, informative course, as well as the teaching staff of past iterations for many parts of the kernel and user code in this repository, allowing me to learn how to construct an operating system in only one semester.



