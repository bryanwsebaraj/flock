#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define exit(...) return __VA_ARGS__

char buf[8192];

int main(int argc, char *argv[])
{
    /**
     * This program serves as a "staller" to demonstrate the
     * use case of the LOCK_NB flag.
     *
     * This process is designed to be run in conjunction with 
     * flocktest_test_nonblocking in flocktest.c
    */
    int fd, con;
    
    printf("=====starting flockstall=====\n\n");

    /* open the file with write only permissions */
    if ((fd = open("flockfile", O_WRONLY)) < 0) {
        printf("ERROR in flockstall: open flockfile failed\n");
        exit();
    }

    /* acquire exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flockstall: could not acquire exclusive lock\n");
        close(fd);
        exit();
    }

    /* signal to flocktest_test_nonblocking that this process has obtained
    an exclusive lock on flockfile */
    produce(1);

    /* write to the file */
    for (int i = 0; i < 50; i++) { // changed from 100 to 50
        if (write(fd, "cccccccccc", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "cccccccccc", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
    }

    /* wait until flocktest_test_nonblocking has tried to obtain
    the exclusive lock (non-blocking) */
    con = consume();

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flockstall: could not release flock!\n");
        close(fd);
        exit();
    }

    close(fd);

    printf("=====finishing flockstall=====\n\n");

    return 0;
}