#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
	int channel[2];
	pipe(channel);
	pid_t pid = fork();
	if (pid == 0) {
		char Hello[6] = "Hello";
		write(channel[1], Hello, sizeof(Hello) - 1);
	} else {
		waitpid(pid, 0, 0);
		if (fork() == 0) { 
			char buffer[6];
			read(channel[0], buffer, sizeof(buffer) - 1);
		}
	}
	return 0;	
}
