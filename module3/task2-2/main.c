#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t last_signal = -1;
volatile sig_atomic_t fds[4096];

void handler_rt(int signum)
{
    last_signal = signum;
}

void handler_rtmin(int signum)
{
    last_signal = signum;
}

int main(int argc, char* argv[])
{
    struct sigaction sigact_rtmin;
    memset(&sigact_rtmin, 0, sizeof(sigact_rtmin));
    sigact_rtmin.sa_handler = handler_rtmin;
    sigact_rtmin.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGRTMIN, &sigact_rtmin, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    struct sigaction sigact_rt;
    memset(&sigact_rt, 0, sizeof(sigact_rt));
    sigact_rt.sa_handler = handler_rt;
    sigact_rt.sa_flags = SA_RESTART;
    for (int signum = SIGRTMIN + 1; signum <= SIGRTMAX; ++signum) {
        if (-1 == sigaction(signum, &sigact_rt, NULL)) {
            perror("Sigaction failed");
            exit(1);
        }
    }

    for (int signum = 1; signum < SIGRTMIN; ++signum) {
        signal(signum, SIG_IGN);
    }

    fflush(stdout);

    for (int number = 0; number < argc - 1; ++number) {
        if (-1 == (fds[number] = open(argv[number + 1], O_RDONLY))) {
            exit(1);
        }
    }

    while (1) {
        pause();
        if (last_signal == SIGRTMIN) {
            break;
        } else if (SIGRTMIN < last_signal && last_signal <= SIGRTMAX)
        {
            char symbol;
            char buffer[10000] = {};
            size_t size = 0;
            while (read(fds[last_signal - SIGRTMIN - 1], &symbol, sizeof(symbol)) > 0) {
                buffer[size] = symbol;
                if (symbol == '\n') {
                    break;
                }
                ++size;
            }
            write(STDOUT_FILENO, buffer, strnlen(buffer, sizeof(buffer)));
        }
    }

    for (int number = 0; number < argc - 1; ++number) {
        close(fds[number]);
    }
    exit(0);
}
