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
    int fd;

    printf("=====starting flockwriter=====\n\n");

    /* open the file with write only permissions */
    fd = open("flockfile", O_WRONLY);

    if (fd < 0) {
        printf("ERROR in flockwriter: open flockfile failed!\n");
        exit();
    }

    /* acquire exclusive lock */
    if (flock(fd, LOCK_EX) == -1) {
        printf("ERROR in flockwriter: could not acquire exclusive lock!\n");
        close(fd);
        exit();
    }

    /* write to the file */
    for (int i = 0; i < 50; i++) { // changed from 100 to 50
        if (write(fd, "jjjjjjjjjj", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
        if (write(fd, "cccccccccc", 10) != 10) {
            printf("error: write aa %d new file failed\n", i);
            exit();
        }
    }

    /* release the exclusive lock */
    if (flock(fd, LOCK_UN) == -1) {
        printf("ERROR in flockwriter: could not release flock!\n");
        close(fd);
        exit();
    }

    close(fd);

    printf("=====finishing flockwriter=====\n");

    return 0;
}