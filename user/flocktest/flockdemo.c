#include <proc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>
#include <file.h>
#include <gcc.h>

#define exit(...) return __VA_ARGS__

char buf[8192];

int main(int arc, char *argv[])
{
    int fd, reader_pid, writer_pid;

    printf("=====flockdemo starting=====\n");

    // /* Begin by creating a file and writing some content to it */
    if ((fd = open("flockfile", O_CREATE | O_RDWR)) < 0) {
        printf("Error: create flockfile failed!\n");
        exit();
    }

    /* acquire an exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR: could not obtain an exclusive lock\n");
        close(fd);
        exit();
    }

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
        printf("ERROR in flocktest_test_downgrade: release shared flock failed!\n");
        close(fd);
        exit();
    }

    close(fd);

    writer_pid = spawn(9, 250);
    reader_pid = spawn(8, 250);

    printf("=====flockdemo finished=====\n");
    return 0;
}