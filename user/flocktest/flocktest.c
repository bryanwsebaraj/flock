#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define exit(...) return __VA_ARGS__

char buf[8192];

/**
 * Test that one thread can have exclusive acccess to flock
*/
void flocktest_test_write(void)
{
    int fd;

    printf("=====flocktest_test_write=====\n");

    fd = open("flockfile", O_CREATE | O_RDWR);

    if (fd < 0) {
        printf("ERROR in flocktest_test_read: create flockfile failed!\n");
        exit();
    }

    /* acquire exclusive lock */    
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flocktest_test_read: acquire exclusive lock failed!\n");
        printf("flocktest_test_read failed.\n");
        exit();
    }

    /* write to the file */
    for (int i = 0; i < 100; i++) {
        if (write(fd, "aaaaaaaaaa", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "bbbbbbbbbb", 10) != 10) {
            printf("error: write bb %d new file failed\n", i);
            exit();
        }
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_write: release exclusive lock failed\n");
        printf("flocktest_test_write failed.\n");
        exit(); 
    }

    close(fd);
    if (unlink("flockfile") < 0) {
        printf("ERROR in flocktest_test_write: unlink flockfile failed\n");
        exit();
    }

    printf("=====flocktest_test_write ok=====\n\n"); 
}


/**
 * Test that one thread can have shared access to flock
*/
void flocktest_test_read(void) 
{
    int fd;

    printf("=====flocktest_test_read=====\n");

    fd = open("flockfile", O_CREATE | O_RDWR);

    if (fd < 0) {
        printf("Error: create flockfile failed!\n");
        printf("flocktest_test_read failed.\n");
        exit();
    } 

    /* acquire shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("ERROR in flocktest_test_read: acquire shared lock failed!\n");
        printf("flocktest_test_read failed.\n");
        exit();
    }

    /* release shared lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_read: release shared lock failed\n");
        printf("flocktest_test_read failed.\n");
        exit();
    }

    close(fd);
    if (unlink("flockfile") < 0) {
        printf("ERROR in flocktest_test_read: unlink flockfile failed\n");
        exit();
    }

    printf("=====flocktest_test_read ok=====\n\n");
}


void flocktest_test_close(void)
{
    int fd;

    printf("=====flocktest_test_close=====\n");

    fd = open("flockfile", O_CREATE | O_RDWR);

    if (fd < 0) {
        printf("Error: create flockfile failed!\n");
        printf("flocktest_test_close failed.\n");
        exit();
    } 

    /* acquire shared lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flocktest_test_close: acquire exclusive lock failed!\n");
        printf("flocktest_test_close failed.\n");
        exit();
    }

    /* close the file WITHOUT releasing the lock */
    close(fd);

    /* reopen the file */
    fd = open("flockfile", O_RDWR);
    
    if (fd < 0) {
        printf("ERROR: create flockfile failed!\n");
        printf("flocktest_test_close failed.\n");
        exit();
    }

    /* check to see if the file is still locked */
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        printf("flocktest_test_close failed: file is still locked\n");
        exit();
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_close: release exclusive lock failed\n");
        printf("flocktest_test_close failed.\n");
        exit(); 
    }

    close(fd); 

    printf("=====flocktest_test_close ok=====\n\n");
}

/**
 * Showcase the use of the LOCK_NB flag
*/
void flocktest_test_nonblocking(void)
{
    int fd, con;

    /* PART 1: can obtain exclusive lock with LOCK_NB flag */
    printf("=====flocktest_test_nonblocking (part 1)=====\n");

    if ((fd = open("flockfile", O_CREATE | O_RDWR)) < 0) {
        printf("ERROR in flocktest_test_nonblocking: create flockfile failed!\n");
        exit();
    }

    /* acquire exclusive lock, non-blocking */
    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        printf("ERROR in flocktest_test_nonblocking: could not acquire exclusive lock (non-blocking)\n");
        close(fd);
        exit();
    }

    /* write something to the file */
    for (int i = 0; i < 50; i++) { // changed from 100 to 50 
        if (write(fd, "aaaaaaaaaa", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "bbbbbbbbbb", 10) != 10) {
            printf("error: write bb %d new file failed\n", i);
            exit();
        }
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_nonblocking: release exclusive lock failed\n");
        exit();
    }

    close(fd);
    printf("=====flocktest_test_nonblocking (part 1) ok=====\n\n");


    /* PART 2: trying to obtain an exclusive lock with LOCK_NB while 
    another thread holds the lock will return EWOULDBLOCK immediately. */
    printf("=====flocktest_test_nonblocking (part 2)=====\n");

    /* spawn flockstall, which will hold an exclusive lock for an extended
    period of time */
    pid_t child_pid;
    if ((child_pid = spawn(10, 500)) == -1) { // flockstall process
        printf("ERROR in flocktest_test_nonblocking: failed to spawn flockstall process\n");
        exit();
    }

    /* wait until flockstall has obtained exclusive lock on flockfile */
    con = consume();

    /* open flockfile */
    if ((fd = open("flockfile", O_RDWR)) < 0) {
        printf("ERROR in flocktest_test_nonblocking: open flockfile failed\n");
        exit();
    }

    /* try to acquire the exclusive lock (non-blocking) */
    int flock_retval;
    if ((flock_retval = flock(fd, LOCK_EX | LOCK_NB)) != -1) {
       printf("flocktest_test_nonblocking failed! value: %d\n", flock_retval); 
       close(fd);
       produce(1);
       exit();
    }

    /* signal to flockstall that this process has already attempted (and failed) to 
    obtain the exclusive lock; now, flockstall can continue */
    produce(1);

    close(fd);
    printf("=====flocktest_test_nonblocking (part 2) ok=====\n\n");
}


/**
 * Showcase upgrading the lock
*/
void flocktest_test_upgrade(void)
{
    int fd;

    printf("=====flocktest_test_upgrade=====\n");

    if ((fd = open("flockfile", O_CREATE | O_RDWR))< 0) {
        printf("ERROR in flocktest_test_upgrade: create flockfile failed.\n");
        exit();
    }

    /* acquire shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("ERROR in flocktest_test_upgrade: could not acquire shared lock\n");
        close(fd);
        exit();
    }

    /* upgrade to an exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flocktest_test_upgrade: could not upgrade to an exclusive lock\n");
        close(fd);
        exit();
    }

    /* write something to the file */
    for (int i = 0; i < 50; i++) { // changed from 100 to 50 
        if (write(fd, "aaaaaaaaaa", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "bbbbbbbbbb", 10) != 10) {
            printf("error: write bb %d new file failed\n", i);
            exit();
        }
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_upgrade: release exclusive lock failed\n");
        exit();
    }

    close(fd);
    printf("=====flocktest_test_upgrade ok=====\n\n");
}


/**
 * Showcase downgrading the lock 
*/
void flocktest_test_downgrade(void)
{
    int fd, i;

    printf("=====flocktest_test_downgrade=====\n");

    if ((fd = open("flockfile", O_CREATE | O_RDWR))< 0) {
        printf("ERROR in flocktest_test_downgrade: create flockfile failed.\n");
        exit();
    }

    /* acquire an exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flocktest_test_downgrade: could not obtain an exclusive lock\n");
        close(fd);
        exit();
    }

    /* write to the file */
    for (int i = 0; i < 100; i++) {
        if (write(fd, "jjjjjjjjjj", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "jjjjjjjjjj", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
    }

    /* release exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flockreader: release shared flock failed!\n");
        close(fd);
        exit();
    }

    close(fd);

    /* reopen flockfile */
    if ((fd = open("flockfile", O_RDWR)) < 0) {
        printf("ERROR in flocktest_test_downgrade: open flockfile failed.\n");
        exit();
    }

    /* acquire an exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flocktest_test_downgrade: could not obtain an exclusive lock\n");
        close(fd);
        exit();
    }

    /* downgrade to a shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("ERROR in flocktest_test_downgrade: could not downgrade to shared lock\n");
        close(fd);
        exit();
    }

    /* read from the file */
    i = read(fd, buf, 100);
    if (i != 100) {
        printf("ERROR in flocktest_test_downgrade: did not read 100 characters from flockfile!\n");
        printf("ERROR: %d\n", i);
        close(fd);
        exit();
    }

    /* print to stdout */
    printf("Read from flockfile: ");
    for (int k = 0; k < 100; k++) {
        printf("%c", buf[k]);
    }
    printf("\n");

    /* release the shared lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flocktest_test_downgrade: release shared flock failed!\n");
        close(fd);
        exit();
    }

    close(fd);
    printf("=====flocktest_test_downgrade ok=====\n\n"); 
}


int main(int argc, char *argv[])
{
    printf("*******flocktests starting*******\n\n");

    flocktest_test_write();
    flocktest_test_read();
    flocktest_test_close();
    flocktest_test_nonblocking();
    flocktest_test_upgrade();
    flocktest_test_downgrade(); 

    printf("*******end of flocktests*******\n\n");


    printf("*******flockdemo starting*******\n\n");

    pid_t flockdemo_pid;
    if ((flockdemo_pid = spawn(7, 1000)) != -1)
        printf("flockdemo in process %d.\n", flockdemo_pid);
    else
        printf("Failed to launch flockdemo.\n");

    return 0; 
}