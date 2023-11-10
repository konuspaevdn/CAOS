#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static void* thread_function(void* arg)
{
	int i = 1;
	while(1) {
		printf("Thread %"PRIu64" is running\n", (uint64_t) arg);
		sleep(1);
		pid_t pid;
		if (i)
		{
			pid = fork();
			i = 0;
		}
		if (pid == 0)
		{
			printf("Child created in thread %"PRIu64" is running\n", (uint64_t) arg);
		}
	}
}

int main()
{
	
	pthread_t threads[2];
	pthread_create(&threads[0], NULL, thread_function, (void*) 1);
	pthread_create(&threads[1], NULL, thread_function, (void*) 2);
	pthread_exit(0);
}
