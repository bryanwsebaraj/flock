#include "shell.h"
#include <stdio.h>

void join_path(char *dst, char *p1, char *p2)
{
    ASSERT(*p2 != '\0');

    // Check for in-place update
    if (dst != p1) {
        strcpy(dst, p1);
    }
    dst += strlen(p1);
    if (*p1 != '\0' && *(dst - 1) != '/') {
        *dst = '/';
        dst++;
    }
    strcpy(dst, p2);
    ASSERT(*(dst + strlen(p2)) == '\0');
}

// Replace path with the first component of the path and return the remainder.
char *split_path(char *path)
{
    char *rest = NULL;

    ASSERT(*path != '/');

    while (*path != '/' && *path != '\0') {
        path++;
    }

    if (*path == '/') {
        *path = '\0';
        rest = path + 1;

        while (*rest == '/' && *rest != '\0') {
            rest++;
        }

        if (*rest == '\0') {
            rest = NULL;
        }
    }

    return rest;
}

void normalize_path(char *path)
{
    int len = strlen(path);
    char *norm = alloca(len + 1);
    char *norm_end = norm;
    char *ret = path;
    char *next;

    ASSERT(*path == '/');
    *norm = '/';
    norm_end++;
    path++;

    while (path != NULL) {
        ASSERT(norm < norm_end);
        ASSERT(*norm == '/');
        next = split_path(path);

        if (strcmp(path, ".") == 0) {
            // Skip
        }
        else if (strcmp(path, "..") == 0) {
            // Remove one dir
            while (norm < norm_end && *(norm_end - 1) != '/') {
                norm_end--;
            }
        }
        else if (*path != '\0') {
            // Copy and add trailing '/'
            if (*(norm_end - 1) != '/') {
                *norm_end = '/';
                norm_end++;
            }
            strcpy(norm_end, path);
            norm_end += strlen(path);
        }

        path = next;
    }

    ASSERT(norm < norm_end);
    ASSERT(*norm == '/');
    *norm_end = '\0';

    ASSERT(strlen(norm) == norm_end - norm);
    ASSERT(norm_end - norm <= len);
    strcpy(ret, norm);
    ASSERT(*(ret + (norm_end - norm)) == '\0');
}
