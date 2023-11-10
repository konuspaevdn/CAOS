#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char* cmd = argv[1];
    char* in_name = argv[2];
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
        int fd_in = open(in_name, O_RDONLY);
        if (fd_in == -1) {
            perror("Open failed");
            exit(1);
        }
        if (dup2(fd_in, 0) == -1) {
            perror("Dup failed");
            exit(1);
        }
        close(fd_in);
        if (dup2(channel[1], 1) == -1) {
            perror("Dup failed");
            exit(1);
        }
        close(channel[1]);
        execlp(cmd, cmd, NULL);
        perror("Execution failed");
        exit(1);
    } else {
        int wstatus;
        waitpid(pid, &wstatus, 0);
        if (close(channel[1]) == -1) {
            perror("Close failed");
            exit(1);
        }
        u_int64_t total_count = 0;
        ssize_t count = 0;
        char buffer[4096];
        while ((count = read(channel[0], buffer, sizeof(buffer))) > 0) {
            total_count += count;
        }
        printf("%lu", total_count);
        close(channel[0]);
    }
    exit(0);
}
