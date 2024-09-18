#include "execute.h"
#include "builtin.h"
#include "command.h"
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "tests/syscall_mock.h"

static void setup_redirection(char *path, int fd, int flags) {
    int file = open(path, flags, S_IRWXU);
    if (file == -1) {
        perror("open file");
        exit(EXIT_FAILURE);
    }
    dup2(file, fd);
    close(file);
}

static char **construct_argv(scommand cmd) {
    int argc = scommand_length(cmd);
    char **argv = malloc((argc + 1) * sizeof(char *)); // +1 para el NULL final

    int i = 0;
    while (!scommand_is_empty(cmd)) {
        argv[i] = strdup(scommand_front(cmd));
        scommand_pop_front(cmd);
        ++i;
    }
    argv[i] = NULL; // execvp espera un NULL al final del arreglo de argumentos

    return argv;
}

static void free_argv(char **argv) {
    for (int i = 0; argv[i] != NULL; ++i) {
        free(argv[i]);
    }
    free(argv);
}

static void execute_scommand(scommand cmd, int in_fd, int out_fd) {
    if (in_fd != STDIN_FILENO) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }

    if (out_fd != STDOUT_FILENO) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }

    char *input_path = scommand_get_redir_in(cmd);
    if (input_path != NULL) {
        setup_redirection(input_path, STDIN_FILENO, O_RDONLY);
    }
    char *output_path = scommand_get_redir_out(cmd);
    if (output_path != NULL) {
        setup_redirection(output_path, STDOUT_FILENO,
                          O_WRONLY | O_CREAT | O_TRUNC);
    }

    char **argv = construct_argv(cmd);
    execvp(argv[0], argv);
    perror("execvp"); // Solo se ejecuta si execvp falla
    free_argv(argv);
    exit(EXIT_FAILURE);
}

static void close_pipe(int pipe_fd) {
    if (pipe_fd != STDIN_FILENO && pipe_fd != STDOUT_FILENO) {
        close(pipe_fd);
    }
}

static void execute_external_pipeline(pipeline apipe) {
    if (!pipeline_get_wait(apipe)) {
        signal(SIGCHLD, SIG_IGN);
    }

    int length = pipeline_length(apipe);
    pid_t pids[length];
    int in_fd = STDIN_FILENO;

    for (int i = 0; i < length; ++i) {
        int pipe_fd[2];
        if (i < length - 1) {
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        } else {
            pipe_fd[0] = STDIN_FILENO;
            pipe_fd[1] = STDOUT_FILENO;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // hijo
            close_pipe(pipe_fd[0]);
            execute_scommand(pipeline_front(apipe), in_fd, pipe_fd[1]);
        } else { // padre
            close_pipe(in_fd);
            close_pipe(pipe_fd[1]);
            in_fd = pipe_fd[0];
            pipeline_pop_front(apipe);
            pids[i] = pid;
        }
    }

    if (pipeline_get_wait(apipe)) {
        for (int i = 0; i < length; ++i) {
            waitpid(pids[i], NULL, 0);
        }
    }
}

void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);

    if (pipeline_is_empty(apipe)) {
        return;
    }

    if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe));
    } else if (!builtin_is_internal(pipeline_front(apipe))) {
        execute_external_pipeline(apipe);
    } else {
        fprintf(stderr, "Builtin command in pipeline is not supported.\n");
        exit(EXIT_FAILURE);
    }
}
