#ifndef _USER_SHELL_H_
#define _USER_SHELL_H_

#include <types.h>

#define alloca(size) __builtin_alloca(size)

typedef enum {
    E_OK = 0,
    E_INVAL_CMD,
    E_FEW_ARGS,
    E_MANY_ARGS,
    E_INVAL_OPT,
    E_INVAL_TYPE,
    E_CANT_OPEN
} err_t;

err_t cmd_ls(int argc, char **argv, int optc, char **optv);
err_t cmd_pwd(int argc, char **argv, int optc, char **optv);
err_t cmd_cd(int argc, char **argv, int optc, char **optv);
err_t cmd_cp(int argc, char **argv, int optc, char **optv);
err_t cmd_mv(int argc, char **argv, int optc, char **optv);
err_t cmd_rm(int argc, char **argv, int optc, char **optv);
err_t cmd_mkdir(int argc, char **argv, int optc, char **optv);
err_t cmd_cat(int argc, char **argv, int optc, char **optv);
err_t cmd_touch(int argc, char **argv, int optc, char **optv);
err_t cmd_write(int argc, char **argv, int optc, char **optv);
err_t cmd_append(int argc, char **argv, int optc, char **optv);

err_t cmd_pathtest(int argc, char **argv, int optc, char **optv);

typedef struct {
    const char *name;
    bool infix;
    int minargs;
    int maxargs;
    err_t (*func)(int argc, char **argv, int optc, char **optv);
} cmd_t;
#define CMD(name, min, max) {#name, FALSE, min, max, cmd_ ## name}
#define INFIX(str, name) {str, TRUE, 2, 2, cmd_ ## name}
#define NULLCMD {"", FALSE, 0, 0, NULL}

void join_path(char *dst, char *p1, char *p2);
char *split_path(char *path);
void normalize_path(char *path);

#endif  /* _USER_SHELL_H_ */
