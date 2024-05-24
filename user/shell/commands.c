#include "shell.h"
#include <file.h>
#include <stdio.h>

// Allocate space for MAX_DIR_DEPTH directories plus '/'s
#define MAX_DIR_DEPTH 10
#define CWD_SIZE      (MAX_DIR_DEPTH * (DIRSIZ + 1))
static char cwd[CWD_SIZE] = "/";

static void update_cwd(char *path)
{
    if (*path == '/') {
        ASSERT(strlen(path) < CWD_SIZE);
        strcpy(cwd, path);
    }
    else {
        ASSERT(strlen(cwd) + strlen(path) + 1 < CWD_SIZE);
        join_path(cwd, cwd, path);
    }

    normalize_path(cwd);
    ASSERT(*cwd == '/');
}

static err_t recurse_dir(char *name, err_t (*cmd)(int argc, char **argv, int optc, char **optv),
                         int optc, char **optv, int dir, char *src, char *dst)
{
    err_t ret = E_OK;
    struct dirent de;
    int argc = 3;
    char *sub_argv[3];
    sub_argv[0] = name;
    sub_argv[1] = alloca(CWD_SIZE);
    if (dst != NULL) {
        sub_argv[2] = alloca(CWD_SIZE);
        argc = 2;
    }

    while (read(dir, (char *) &de, sizeof(struct dirent)) == sizeof(struct dirent)) {
        if (de.inum != 0 && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
            join_path(sub_argv[1], src, de.name);
            if (dst != NULL) {
                join_path(sub_argv[2], dst, de.name);
            }
            if ((ret = (*cmd)(argc, sub_argv, optc, optv)) != E_OK) {
                return ret;
            }
        }
    }

    return ret;
}

err_t cmd_ls(int argc, char **argv, int optc, char **optv)
{
    char *path;
    struct file_stat stat;
    int fd;
    struct dirent de;
    bool all = FALSE;

    for (optc--; optc >= 0; optc--) {
        switch (optv[optc][1]) {
        case 'a':
            all = TRUE;
            break;
        default:
            return E_INVAL_OPT;
        }
    }

    argv++;  // Skip command
    argc--;
    do {
        // Default to current directory
        if (argc == 0) {
            path = ".";
        }
        else {
            path = *argv;
        }

        if ((fd = open(path, O_RDONLY)) == -1) {
            return E_CANT_OPEN;
        }
        ASSERT(fstat(fd, &stat) != -1);

        switch (stat.type) {
        case T_FILE:
            // Print file name
            printf("%s\n", path);
            break;
        case T_DIR:
            // Print directory contents
            while (read(fd, (char *) &de, sizeof(struct dirent)) == sizeof(struct dirent)) {
                if (de.inum != 0 && (all || de.name[0] != '.')) {
                    printf("%s\n", de.name);
                }
            }
            break;
        default:
            ASSERT(close(fd) != -1);
            return E_INVAL_TYPE;
        }

        ASSERT(close(fd) != -1);
        argc--;
        argv++;
    } while (argc > 0);

    return E_OK;
}

err_t cmd_pwd(int argc, char **argv, int optc, char **optv)
{
    if (optc > 0) {
        return E_INVAL_OPT;
    }

    printf("%s\n", cwd);

    return E_OK;
}

err_t cmd_cd(int argc, char **argv, int optc, char **optv)
{
    char *path;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    // Default to root directory
    if (argc == 1) {
        path = "/";
    }
    else {
        path = argv[1];
    }

    if (chdir(path) == -1) {
        return E_INVAL_TYPE;
    }

    update_cwd(path);

    return E_OK;
}

err_t cmd_cp(int argc, char **argv, int optc, char **optv)
{
    char *src, *dst;
    int srcfd, dstfd = -1;
    struct file_stat stat;
    int len;
    char buf[1024];
    err_t ret = E_OK;
    bool recurse = FALSE;

    for (optc--; optc >= 0; optc--) {
        switch (optv[optc][1]) {
        case 'r':
            recurse = TRUE;
            break;
        default:
            return E_INVAL_OPT;
        }
    }

    src = argv[1];
    dst = argv[2];

    if ((srcfd = open(src, O_RDONLY)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(srcfd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        ASSERT((dstfd = open(dst, O_CREATE | O_RDWR)) != -1);
        while ((len = read(srcfd, buf, 1024)) > 0) {
            ASSERT(write(dstfd, buf, len) == len);
        }
        break;
    case T_DIR:
        if (!recurse) {
            ret = E_INVAL_TYPE;
            goto end;
        }
        ASSERT((dstfd = mkdir(dst)) != -1);
        if ((ret = recurse_dir("cp", &cmd_cp, optc, optv, srcfd, src, dst)) != E_OK) {
            goto end;
        }
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

end:
    ASSERT(close(srcfd) != -1);
    if (dstfd > 0) {
        ASSERT(close(dstfd) != -1);
    }
    return ret;
}

err_t cmd_mv(int argc, char **argv, int optc, char **optv)
{
    char *src, *dst;
    int fd;
    struct file_stat stat;
    err_t ret = E_OK;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    src = argv[1];
    dst = argv[2];

    if ((fd = open(src, O_RDONLY)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(fd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        ASSERT(link(src, dst) != -1);
        break;
    case T_DIR:
        ASSERT(mkdir(dst) != -1);
        if ((ret = recurse_dir("mv", &cmd_mv, optc, optv, fd, src, dst)) != E_OK) {
            goto end;
        }
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

    ASSERT(unlink(src) != -1);

end:
    ASSERT(close(fd) != -1);
    return ret;
}

err_t cmd_rm(int argc, char **argv, int optc, char **optv)
{
    char *path;
    int fd;
    struct file_stat stat;
    err_t ret = E_OK;
    bool recurse = FALSE;

    for (optc--; optc >= 0; optc--) {
        switch (optv[optc][1]) {
        case 'r':
            recurse = TRUE;
            break;
        default:
            return E_INVAL_OPT;
        }
    }

    path = argv[1];

    if ((fd = open(path, O_RDONLY)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(fd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        break;
    case T_DIR:
        if (!recurse) {
            ret = E_INVAL_TYPE;
            goto end;
        }
        if ((ret = recurse_dir("rm", &cmd_rm, optc, optv, fd, path, NULL)) != E_OK) {
            goto end;
        }
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

    ASSERT(unlink(path) != -1);

end:
    ASSERT(close(fd) != -1);
    return ret;
}

err_t cmd_mkdir(int argc, char **argv, int optc, char **optv)
{
    char *path;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    path = argv[1];

    ASSERT(mkdir(path) != -1);

    return E_OK;
}

err_t cmd_cat(int argc, char **argv, int optc, char **optv)
{
    char *path;
    int fd;
    struct file_stat stat;
    char buf[1024];
    int len;
    err_t ret = E_OK;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    path = argv[1];

    if ((fd = open(path, O_RDONLY)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(fd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        while ((len = read(fd, buf, 1024)) > 0) {
            puts(buf, len);
        }
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

end:
    ASSERT(close(fd) != -1);
    return ret;
}

err_t cmd_touch(int argc, char **argv, int optc, char **optv)
{
    char *path;
    int fd;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    path = argv[1];

    if ((fd = open(path, O_RDONLY)) == -1) {
        if ((fd = open(path, O_CREATE)) == -1) {
            return E_CANT_OPEN;
        }
    }

    ASSERT(close(fd) != -1);

    return E_OK;
}

err_t cmd_write(int argc, char **argv, int optc, char **optv)
{
    char *text;
    char *path;
    int fd;
    struct file_stat stat;
    err_t ret = E_OK;

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    text = argv[0];
    path = argv[2];

    // Recreate file if it exists
    if ((fd = open(path, O_RDONLY)) != -1) {
        ASSERT(fstat(fd, &stat) != -1);
        ASSERT(unlink(path) != -1);
        ASSERT(close(fd) != -1);
    }
    if ((fd = open(path, O_CREATE | O_WRONLY)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(fd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        ASSERT(write(fd, text, strlen(text)) == strlen(text));
        ASSERT(write(fd, "\n", 1) == 1);
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

end:
    ASSERT(close(fd) != -1);
    return ret;
}

err_t cmd_append(int argc, char **argv, int optc, char **optv)
{
    char *text;
    char *path;
    int fd;
    struct file_stat stat;
    err_t ret = E_OK;
    char buf[1024];

    if (optc > 0) {
        return E_INVAL_OPT;
    }

    text = argv[0];
    path = argv[2];

    if ((fd = open(path, O_CREATE | O_RDWR)) == -1) {
        return E_CANT_OPEN;
    }
    ASSERT(fstat(fd, &stat) != -1);

    switch (stat.type) {
    case T_FILE:
        // Seek to end of file
        while (read(fd, buf, 1024) > 0) {}
        ASSERT(write(fd, text, strlen(text)) == strlen(text));
        ASSERT(write(fd, "\n", 1) == 1);
        break;
    default:
        ret = E_INVAL_TYPE;
        goto end;
    }

end:
    ASSERT(close(fd) != -1);
    return ret;
}

err_t cmd_pathtest(int argc, char **argv, int optc, char **optv)
{
    char path1[1024], path2[1024], path3[1024];
    char *path4, *path5;

    strcpy(path1, "d1/d2");
    strcpy(path2, "f.txt");
    join_path(path3, path1, path2);
    ASSERT(strcmp(path3, "d1/d2/f.txt") == 0);

    join_path(path1, path1, path3);
    ASSERT(strcmp(path1, "d1/d2/d1/d2/f.txt") == 0);

    strcpy(path3, "/");
    join_path(path1, path3, path2);
    ASSERT(strcmp(path1, "/f.txt") == 0);

    strcpy(path3, "");
    join_path(path1, path3, path2);
    ASSERT(strcmp(path1, "f.txt") == 0);

    strcpy(path1, "d1/./d2/../f.txt");
    path5 = path1;
    path4 = split_path(path5);
    ASSERT(strcmp(path5, "d1") == 0);
    ASSERT(strcmp(path4, "./d2/../f.txt") == 0);

    path5 = path4;
    path4 = split_path(path5);
    ASSERT(strcmp(path5, ".") == 0);
    ASSERT(strcmp(path4, "d2/../f.txt") == 0);

    path5 = path4;
    path4 = split_path(path5);
    ASSERT(strcmp(path5, "d2") == 0);
    ASSERT(strcmp(path4, "../f.txt") == 0);

    path5 = path4;
    path4 = split_path(path5);
    ASSERT(strcmp(path5, "..") == 0);
    ASSERT(strcmp(path4, "f.txt") == 0);

    path5 = path4;
    path4 = split_path(path5);
    ASSERT(strcmp(path5, "f.txt") == 0);
    ASSERT(path4 == NULL);

    strcpy(path5, "");
    path4 = split_path(path5);
    ASSERT(strcmp(path5, "") == 0);
    ASSERT(path4 == NULL);

    strcpy(path1, "/d1/./d2///../././d3/");
    normalize_path(path1);
    ASSERT(strcmp(path1, "/d1/d3") == 0);

    strcpy(path1, "/d1/..");
    normalize_path(path1);
    ASSERT(strcmp(path1, "/") == 0);

    strcpy(path1, "/..");
    normalize_path(path1);
    ASSERT(strcmp(path1, "/") == 0);

    strcpy(path1, "/./../");
    normalize_path(path1);
    ASSERT(strcmp(path1, "/") == 0);

    strcpy(path1, "/../.");
    normalize_path(path1);
    ASSERT(strcmp(path1, "/") == 0);

    return E_OK;
}
