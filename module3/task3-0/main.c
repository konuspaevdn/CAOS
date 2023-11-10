#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int sock;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Can't create a socket");
        exit(1);
    }
    in_port_t port = htons((short)strtol(argv[2], NULL, 10));
    struct in_addr ip;
    ip.s_addr = inet_addr(argv[1]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = ip;
    if (-1 == connect(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("Can't connect to the server");
        exit(1);
    }
    int send_num = 0;
    int receive_num = 0;
    while (scanf("%d", &send_num) != EOF) {
        if (-1 == send(sock, &send_num, sizeof(send_num), 0)) {
            break;
        }
        if (recv(sock, &receive_num, sizeof(receive_num), 0) <= 0) {
            break;
        }
        printf("%d\n", receive_num);
    }
    if (-1 == shutdown(sock, SHUT_RDWR))
    {
    	perror("Server shutdown failed");
    }
    close(sock);
    exit(0);
}
