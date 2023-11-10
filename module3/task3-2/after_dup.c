#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
	int fd = open("draft.txt", O_CREAT, 0640);
	pid_t pid;
	if (0 == (pid = fork()))
	{
		dup2(fd, STDOUT_FILENO);
	} else
	{
		int wstatus;
		waitpid(pid, &wstatus, 0);
		printf("Writing here\n");
	}
	exit(0);
}
