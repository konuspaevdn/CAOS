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
    int counter;
    int flag = -1;  // to switch pipe ends for a current command
    for (counter = 1; counter < argc; ++counter) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            exit(1);
        }
        flag = (flag + 1) % 2;
        if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status)) {
                printf("Child failure\n");
                exit(1);
            }
            char buffer[65536];
            int old_flags;
            switch (flag) {
                case 0:
                    //old_flags = fcntl(pipe_1[0], F_GETFL);
                    //fcntl(pipe_1[0], F_SETFL, old_flags | O_NONBLOCK);
                    while (read(pipe_1[0], buffer, sizeof(buffer)) > 0) {
                    }
                    //fcntl(pipe_1[0], F_SETFL, old_flags);
                    break;
                case 1:
                    //old_flags = fcntl(pipe_2[0], F_GETFL);
                    //fcntl(pipe_2[0], F_SETFL, old_flags | O_NONBLOCK);
                    while (read(pipe_2[0], buffer, sizeof(buffer)) > 0) {
                    }
                    //fcntl(pipe_2[0], F_SETFL, old_flags);
                    break;
            }
        } else {
            break;
        }
    }
    if (counter == argc) {
        exit(0);
    }
    switch(flag) {
        case 0:
            close(pipe_1[1]);
            close(pipe_2[0]);
            if (counter != 1) {
                if (dup2(pipe_1[0], STDIN_FILENO) == -1) {
                    perror("Dup failed");
                    exit(1);
                }
                close(pipe_1[0]);
            }
            if (counter != argc - 1) {
                if (dup2(pipe_2[1], STDOUT_FILENO) == -1) {
                    perror("Dup failed");
                    exit(1);
                }
                close(pipe_2[1]);
            }
            break;
        case 1:
            close(pipe_2[1]);
            close(pipe_1[0]);
            if (dup2(pipe_2[0], STDIN_FILENO) == -1) {
                perror("Dup failed");
                exit(1);
            }
            close(pipe_2[0]);
            if (counter != argc - 1) {
                if (dup2(pipe_1[1], STDOUT_FILENO) == -1) {
                    perror("Dup failed");
                    exit(1);
                }
                close(pipe_1[1]);
            }
            break;
    }
    execlp(argv[counter], argv[counter], NULL);
    perror("Execution failed");
    exit(1);
}
