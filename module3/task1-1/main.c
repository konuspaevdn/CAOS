#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char* cmd1 = argv[1];
    char* cmd2 = argv[2];
    int channel[2];
    if (pipe(channel) == -1) {
        perror("Pipe failed");
        exit(1);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }
    if (pid == 0) {
        close(channel[0]);
        if (dup2(channel[1], 1) == -1) {
            perror("Dup failed");
            exit(1);
        }
        close(channel[1]);
        execlp(cmd1, cmd1, NULL);
        perror("Execution failed");
        exit(1);
    } else {
        if (close(channel[1]) == -1) {
            perror("Close failed");
            exit(1);
        }
        int wstatus;
        waitpid(pid, &wstatus, 0);
        if (WEXITSTATUS(wstatus) != 0) {
            printf("Child process finished with an error\n");
            exit(1);
        }
        if (dup2(channel[0], 0) == -1) {
            perror("Dup failed");
            exit(1);
        }
        close(channel[0]);
        execlp(cmd2, cmd2, NULL);
        perror("Execution failed");
        exit(1);
    }
    exit(0);
}
