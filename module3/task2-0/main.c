#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t count = 0;
volatile sig_atomic_t if_term = 0;

void handler_int(int signum)
{
	count++;
}

void handler_term(int signum)
{
	if_term = 1;
}

int main()
{
	struct sigaction sigact_int;
	memset(&sigact_int, 0, sizeof(sigact_int));
	sigact_int.sa_handler = handler_int;
	sigact_int.sa_flags = SA_RESTART;
	if (-1 == sigaction(SIGINT, &sigact_int, NULL))
	{
		perror("Sigaction failed");
		exit(1);
	}
	
	struct sigaction sigact_term;
	memset(&sigact_term, 0, sizeof(sigact_term));
	sigact_term.sa_handler = handler_term;
	sigact_term.sa_flags = SA_RESTART;
	if (-1 == sigaction(SIGTERM, &sigact_term, NULL))
	{
		perror("Sigaction failed");
		exit(1);
	}
	
	printf("%d\n", getpid());
	fflush(stdout);
	
	while (!if_term)
	{
		pause();
	}
	
	printf("%d\n", count);
	exit(0);
}
