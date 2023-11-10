#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

void addToEpoll(int epoll_fd, int fdes)
{
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLHUP;
	event.data.fd = fdes;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fdes, &event);
}

int main(int argc, char* argv[])
{
	int sock;
	if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0))) {
        	perror("Can't create a socket");
        	exit(1);
    	}
    	int flags = fcntl(sock, F_GETFL);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    	in_port_t port = htons((short)strtol(argv[1], NULL, 10));
    	struct in_addr ip;
    	ip.s_addr = inet_addr("127.0.0.1");
    	struct sockaddr_in addr;
    	addr.sin_family = AF_INET;
    	addr.sin_port = port;
    	addr.sin_addr = ip;
    	if (-1 == bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        	perror("Bind failed");
        	exit(1);
    	}
    	if (-1 == listen(sock, SOMAXCONN)) {
        	perror("Listen failed");
        	exit(1);
    	}
    	
    	int epoll_fd = epoll_create1(0);
    	addToEpoll(epoll_fd, sock);
    	
    	struct epoll_event events[1000];
    	while (1)
    	{
    		int events_pending = epoll_wait(epoll_fd, events, sizeof(events) / sizeof(*events), 500);
    		printf("Events pending: %d\n", events_pending);
    		for (size_t i = 0; i < events_pending; ++i)
    		{
    			const struct epoll_event event = events[i];
    			int readyToRead = event.events & EPOLLIN;
    			int errorOccured = event.events & EPOLLERR;
    			int disconnected = event.events & EPOLLHUP;
    			int obj = event.data.fd;
    		
    			int client;
    			if (obj == sock && readyToRead)
    			{
    				if (-1 == (client = accept(sock, NULL, NULL))) 
    				{
			    		perror("Connection failed");
			    		continue;
				}
				printf("Everything is fine\n");
				addToEpoll(epoll_fd, client);
				int cl_flags = fcntl(client, F_GETFL);
				fcntl(client, F_SETFL, cl_flags | O_NONBLOCK);
					
    			} else
    			{
    				if (readyToRead) {
    					printf("Hell yeah\n");
	    				char buffer[4096] = {};
	    				char symbol;
	    				size_t idx = 0;
	    				while (recv(client, &symbol, sizeof(symbol), 0) > 0)
    					{
    						buffer[idx] = symbol;
    						++idx;
    					}
    					printf("%s\n", buffer);
    					shutdown(sock, SHUT_RDWR);
    				}
    				if (disconnected)
    				{
    					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client, NULL);
    					// printf("Client exited\n");
    					shutdown(client, SHUT_RDWR);
        				close(client);
    				}
    			}
    		}
        }
        shutdown(sock, SHUT_RDWR);
        close(sock);
	exit(0);
}
