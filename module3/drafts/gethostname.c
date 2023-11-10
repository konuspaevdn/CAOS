#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


int main(int argc, char* argv[])
{
	char buffer[100] = {};
	struct hostent* host_entry;
	gethostname(buffer, 100);
	host_entry = gethostbyname(buffer);
	char* ip_buffer;
	ip_buffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
	char ip[16] = {};
	snprintf(ip, sizeof(ip), "%s", ip_buffer);
	printf("%s\n", ip);
	return 0;
}
