#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t N = -1;
volatile sig_atomic_t pid = 0;

void action_rtmin(int signum, siginfo_t* info, void* context)
{
    N = info->si_value.sival_int;
    pid = info->si_pid;
}

int main()
{
    struct sigaction sigact_rtmin;
    memset(&sigact_rtmin, 0, sizeof(sigact_rtmin));
    sigact_rtmin.sa_sigaction = action_rtmin;
    sigact_rtmin.sa_flags = SA_RESTART | SA_SIGINFO;
    if (-1 == sigaction(SIGRTMIN, &sigact_rtmin, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    for (int signum = 1; signum < SIGRTMAX; ++signum) {
        if (signum != SIGRTMIN) {
            signal(signum, SIG_IGN);
        }
    }

    while (1) {
        pause();
        if (N == 0) {
            break;
        }
        union sigval sigval;
        sigval.sival_int = N - 1;
        if (-1 == sigqueue(pid, SIGRTMIN, sigval)) {
            perror("Sigqueue is full");
            exit(1);
        }
    }
    exit(0);
}