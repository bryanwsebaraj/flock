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
    int fd, i;

    printf("=====starting flockreader=====\n\n");

    /* open the file with read only permissions */
    fd = open("flockfile", O_RDONLY);

    if (fd < 0) {
        printf("ERROR in flockreader: open flockfile failed!\n");
        exit();
    }

    /* acquire shared lock */
    if (flock(fd, LOCK_SH) == -1) {
        printf("ERROR in flockreader: acquired shared flock failed!\n");
        close(fd);
        exit();
    }

    /* read the first 100 characters from the file */
    i = read(fd, buf, 100);
    if (i != 100) {
        printf("ERROR in flockreader: did not read 100 characters from flockfile!\n");
        flock(fd, LOCK_UN);
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
        printf("ERROR in flockreader: release shared flock failed!\n");
        close(fd);
        exit();
    }

    close(fd);

    printf("=====finished flockreader=====\n\n");

    return 0;
}