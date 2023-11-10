#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t if_int_term = 0;
volatile sig_atomic_t last_signal = -1;

void handler_usr1(int signum)
{
    last_signal = SIGUSR1;

}

void handler_usr2(int signum)
{
    last_signal = SIGUSR2;
}

void handler_int_term(int signum)
{
    if_int_term = 1;
}

int main()
{
    struct sigaction sigact_usr1;
    memset(&sigact_usr1, 0, sizeof(sigact_usr1));
    sigact_usr1.sa_handler = handler_usr1;
    sigact_usr1.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGUSR1, &sigact_usr1, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    struct sigaction sigact_usr2;
    memset(&sigact_usr2, 0, sizeof(sigact_usr2));
    sigact_usr2.sa_handler = handler_usr2;
    sigact_usr2.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGUSR2, &sigact_usr2, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    struct sigaction sigact_int_term;
    memset(&sigact_int_term, 0, sizeof(sigact_int_term));
    sigact_int_term.sa_handler = handler_int_term;
    sigact_int_term.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGINT, &sigact_int_term, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }
    if (-1 == sigaction(SIGTERM, &sigact_int_term, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    printf("%d\n", getpid());
    fflush(stdout);

    int value;
    scanf("%d", &value);

    while (!if_int_term) {
        pause();
        char buffer[6] = {};
        if (last_signal == SIGUSR1) {
            ++value;
        } else
        {
            value *= -1;
        }
        snprintf(buffer, sizeof(buffer), "%d\n", value);
        write(STDOUT_FILENO, buffer, strnlen(buffer, sizeof(buffer)));
        fflush(stdout);
    }
    exit(0);
}
