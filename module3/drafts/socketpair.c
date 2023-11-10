#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main()
{
	int sv[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	
	char Hello[7] = "Hello\n";
	send(sv[0], Hello, strnlen(Hello, sizeof(Hello)), 0);
	char buffer[10] = {};
	recv(sv[1], buffer, sizeof(buffer), 0);
	printf("%s", buffer);
	return 0;
}
