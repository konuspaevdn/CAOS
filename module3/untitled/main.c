#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc == 1) {
        exit(0);
    }
    int pipe_1[2];
    int pipe_2[2];
    if (pipe(pipe_1) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    if (pipe(pipe_2) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    int counter = 0;
    pid_t pid = -1;
    int flag = -1;
    while (pid != 0) {
        ++counter;
        flag = (flag + 1) % 2;
        if (counter == argc - 1) {
            break;
        }
        pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            exit(1);
        }
        int status;
        if (pid > 0) {
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status) != 0) {
                perror("Child exited with an error");
                exit(1);
            }
        }
    }
    if (pid == 0) {
        if (flag == 0) {
            close(pipe_2[0]);
            close(pipe_1[1]);
            /*int old_flags = fcntl(pipe_1[0], F_GETFL);
            fcntl(pipe_1[0], F_SETFL, old_flags | O_NONBLOCK);
            char buffer[65536];
            while (read(pipe_1[0], buffer, sizeof(buffer)) > 0) {
            }*/
            if (counter != 1) {
                if (dup2(pipe_1[0], STDIN_FILENO) == -1) {
                    perror("Dup failed");
                    exit(1);
                }
                close(pipe_1[0]);
            }
            if (dup2(pipe_2[1], STDOUT_FILENO) == -1) {
                perror("Dup failed");
                exit(1);
            }
            close(pipe_2[1]);
        } else {
            close(pipe_1[0]);
            close(pipe_2[1]);
            /*int old_flags = fcntl(pipe_2[0], F_GETFL);
            fcntl(pipe_2[0], F_SETFL, old_flags | O_NONBLOCK);
            char buffer[65536];
            while (read(pipe_2[0], buffer, sizeof(buffer)) > 0) {
            }*/
            if (counter != 1) {
                if (dup2(pipe_2[0], STDIN_FILENO) == -1) {
                    perror("Dup failed");
                    exit(1);
                }
                close(pipe_2[0]);
            }
            if (dup2(pipe_1[1], STDOUT_FILENO) == -1) {
                perror("Dup failed");
                exit(1);
            }
            close(pipe_1[1]);
        }
        execlp(argv[counter], argv[counter], NULL);
        perror("Execution failed");
        exit(1);
    } else {
        close(pipe_1[1]);
        close(pipe_2[1]);
        if (flag == 0) {
            close(pipe_2[0]);
            /*int old_flags = fcntl(pipe_1[0], F_GETFL);
            fcntl(pipe_1[0], F_SETFL, old_flags | O_NONBLOCK);
            char buffer[65536];
            while (read(pipe_1[0], buffer, sizeof(buffer)) > 0) {
            }*/
            if (dup2(pipe_1[0], STDIN_FILENO) == -1) {
                perror("Dup failed");
                exit(1);
            }
            close(pipe_1[0]);
        } else {
            close(pipe_1[0]);
            /*int old_flags = fcntl(pipe_2[0], F_GETFL);
            fcntl(pipe_2[0], F_SETFL, old_flags | O_NONBLOCK);
            char buffer[65536];
            while (read(pipe_2[0], buffer, sizeof(buffer)) > 0) {
            }*/
            if (dup2(pipe_2[0], STDIN_FILENO) == -1) {
                perror("Dup failed");
                exit(1);
            }
            close(pipe_2[0]);
        }
        execlp(argv[counter], argv[counter], NULL);
        perror("Execution failed");
        exit(1);
    }
}
