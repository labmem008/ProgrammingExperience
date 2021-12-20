#include "myshell_parser.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// this function handles normal process
void handle_child(pid_t pid, int* wstatus) {
    
    waitpid(pid, wstatus, 0);
}

// this function handles background process
void handle_background_child() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT; // so that no zombie is created
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("ERROR");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    char input[MAX_LINE_LENGTH];
    while (true) {
        if (!(argv[1] && !strcmp(argv[1], "-n")))
            printf("my_shell$");
        char* s = fgets(input, MAX_LINE_LENGTH, stdin);
        if (feof(stdin) || s == NULL) return 0;
        if (strcmp(s, "\n") == 0) continue;

        // build pipeline
        struct pipeline* my_pipeline = pipeline_build(input);
        bool is_background = my_pipeline->is_background;
        struct pipeline_command* my_pipeline_command = my_pipeline->commands;
        int pipeline_command_index = 1;
        pid_t pid = -1;
        int wstatus;
        int flag = 0;
        int fd[2];
        if (pipe(fd) == -1) {
            perror("error when creating pipe");
            exit(1);
        }
        while (my_pipeline_command) {
            char* in_path = my_pipeline_command->redirect_in_path;
            char* out_path = my_pipeline_command->redirect_out_path;
            pid = fork();
            if (pid == 0) {
                // child process

                if (in_path != NULL) {
                    // redirect-in exist
                    int in_path_dup = open(in_path, O_RDWR, S_IRUSR | S_IWUSR);
                    if (in_path_dup == -1) {
                        perror("ERROR: Input File Open");
                        exit(1);
                    }
                    close(STDIN_FILENO);
                    if (dup2(in_path_dup, STDIN_FILENO) == -1) {
                        perror("ERROR: dup2-69");
                        exit(1);
                    }
                    close(in_path_dup);

                    if (out_path != NULL) {
                        // redirect-out exist
                        int out_path_dup = open(out_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                        if (out_path_dup == -1) {
                            perror("ERROR: Output File Open");
                            exit(1);
                        }
                        close(STDOUT_FILENO);
                        if (dup2(out_path_dup, STDOUT_FILENO) == -1) {
                            perror("ERROR: dup2-83");
                            exit(1);
                        }
                        close(out_path_dup);
                    } else if (my_pipeline_command->next != NULL) {
                        close(fd[0]);
                        if (dup2(fd[1], STDOUT_FILENO) == -1) {
                            perror("ERROR: dup2-90");
                            exit(1);
                        }
                        close(fd[1]);
                    }
                } else if (pipeline_command_index == 1) {
                    // this is the first command
                    if (out_path != NULL) {
                        int out_path_dup = open(out_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                        if (out_path_dup == -1) {
                            perror("ERROR: Output File Open");
                            exit(1);
                        }
                        close(STDOUT_FILENO);
                        if (dup2(out_path_dup, STDOUT_FILENO) == -1) {
                            perror("ERROR: dup2-105");
                            exit(1);
                        }
                        close(out_path_dup);
                    } else if (my_pipeline_command->next != NULL) {
                        // the first command && not the last command
                        close(fd[0]);
                        if (dup2(fd[1], STDOUT_FILENO) == -1) {
                            perror("ERROR: dup2-113");
                            exit(1);
                        }
                        close(fd[1]);
                    } else {} // this is the only command, do nothing
                } else if (out_path != NULL) { 
                    // redirect-out exist && redirect-in does not exist
                    close(fd[1]);
                    if (dup2(fd[0], STDIN_FILENO) == -1) {
                        perror("ERROR: dup2-122");
                        exit(1);
                    }
                    close(fd[0]);
                    int out_path_dup = open(out_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                    if (out_path_dup == -1) {
                        perror("ERROR: Output File Open");
                        exit(1);
                    }
                    close(STDOUT_FILENO);
                    if (dup2(out_path_dup, STDOUT_FILENO) == -1) {
                        perror("ERROR: dup2-133");
                        exit(1);
                    }
                    close(out_path_dup);
                } else if (my_pipeline_command->next != NULL) { 
                    // redirection does not exist && not the last command
                    close(fd[1]);
                    if (dup2(fd[0], STDIN_FILENO) == -1) {
                        perror("ERROR: dup2-141");
                        exit(1);
                    }
                    close(fd[0]);
                    close(fd[0]);
                    if (dup2(fd[1], STDOUT_FILENO) == -1) {
                        perror("ERROR: dup2-147");
                        close(fd[1]);
                        exit(1);
                    }
                    close(fd[1]);
                } else {
                    // this is when the command is in between of two commands
                    close(fd[1]);
                    if (dup2(fd[0], STDIN_FILENO) == -1) {
                        perror("ERROR: dup2-156");
                        exit(1);
                    }
                    close(fd[0]);
                }
                if (execvp(my_pipeline_command->command_args[0], my_pipeline_command->command_args) == -1) {
                    fprintf(stderr, "ERROR: %s: %s\n", my_pipeline_command->command_args[0], strerror(errno));
                    close(fd[0]);
                    flag = 1; // set the error flag
                    exit(1);
                }
            } else {
                if (flag == 1) {
                    printf("ERROR: Unknown Error");
                    handle_background_child();
                }
                my_pipeline_command = my_pipeline_command->next;
                pipeline_command_index += 1;
            }
        }
        close(fd[0]);
        close(fd[1]);
        if (is_background) {
            handle_background_child();
        } else {
            handle_child(pid, &wstatus);
        }
        pipeline_free(my_pipeline);
    }
}
