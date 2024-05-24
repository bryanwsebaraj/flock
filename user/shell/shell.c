#include "shell.h"
#include <gcc.h>
#include <stdio.h>
#include <syscall.h>
#include <x86.h>

#define CMDLEN 100

static const char *errstrs[] = {
    [E_INVAL_CMD] = "Invalid command",
    [E_FEW_ARGS] = "Too few arguments",
    [E_MANY_ARGS] = "Too many arguments",
    [E_INVAL_OPT] = "Invalid option",
    [E_INVAL_TYPE] = "Invalid file type",
    [E_CANT_OPEN] = "Failed to open file"
};
#define NERRORS (sizeof(errstrs) / sizeof(const char *))

static cmd_t commands[] = {
    CMD(ls, 0, -1),
    CMD(pwd, 0, 0),
    CMD(cd, 0, 1),
    CMD(cp, 2, 2),
    CMD(mv, 2, 2),
    CMD(rm, 1, 1),
    CMD(mkdir, 1, 1),
    CMD(cat, 1, 1),
    CMD(touch, 1, 1),
    INFIX(">", write),
    INFIX(">>", append),
    CMD(pathtest, 0, 0),
    NULLCMD
};

static void print_error(err_t err)
{
    ASSERT(err < NERRORS);
    const char *str = errstrs[err];
    ASSERT(str != NULL);

    printf("Error: %s.\n", str);
}

static void count_tokens(char *line, int *argc, int *optc)
{
    *argc = 0;
    *optc = 0;

    while (*line != '\0') {
        // Skip leading space
        while (*line == ' ') {
            line++;
        }

        if (*line == '-') {
            // Option
            *optc += 1;
        }
        else if (*line != '\0') {
            // Argument
            *argc += 1;
        }

        // Skip to next token
        while (*line != ' ' && *line != '\0') {
            line++;
        }
    }
}

static void tokenize(char *line, char **argv, char **optv)
{
    while (*line != '\0') {
        // Skip leading space
        while (*line == ' ') {
            line++;
        }

        if (*line == '-') {
            // Option
            *optv = line;
            optv++;
        }
        else if (*line != '\0') {
            // Argument
            *argv = line;
            argv++;
        }

        // Skip to next token
        while (*line != ' ' && *line != '\0') {
            line++;
        }

        // Replace delimiter with '\0'
        if (*line == ' ') {
            *line = '\0';
            line++;
        }
    }
}

static err_t process_cmd(char *line)
{
    int argc, optc;
    char **argv, **optv;
    cmd_t *cmd;
    int arg_idx;

    count_tokens(line, &argc, &optc);
    argv = alloca(argc * sizeof(char *));
    optv = alloca(optc * sizeof(char *));
    tokenize(line, argv, optv);

    if (argc == 0) {
        return (optc == 0) ? E_OK : E_INVAL_CMD;
    }

    for (cmd = commands; cmd->func != NULL; cmd++) {
        arg_idx = cmd->infix ? 1 : 0;
        if (arg_idx >= argc) {
            continue;
        }

        if (strcmp(argv[arg_idx], cmd->name) == 0) {
            if (cmd->minargs > argc - 1) {
                return E_FEW_ARGS;
            }
            if (0 < cmd->maxargs && cmd->maxargs < argc - 1) {
                return E_MANY_ARGS;
            }
            return cmd->func(argc, argv, optc, optv);
        }
    }

    return E_INVAL_CMD;
}

int main(int argc, char *argv[])
{
    char line[CMDLEN];
    err_t ret;

    ASSERT(close(open(".", O_RDONLY)) != -1);

    while (1) {
        readline(line, CMDLEN);
        if ((ret = process_cmd(line)) != 0) {
            print_error(ret);
        }
    }
}
