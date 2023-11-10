#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
    int sock;
    if (-1 == (sock = socket(AF_INET, SOCK_DGRAM, 0))) {
        perror("Can't create a socket");
        exit(1);
    }
    in_port_t port = htons((short)strtol(argv[1], NULL, 10));
    struct in_addr ip;
    ip.s_addr = inet_addr("127.0.0.1");
    struct sockaddr_in destination;
    destination.sin_family = AF_INET;
    destination.sin_port = port;
    destination.sin_addr = ip;
    int send_buffer;
    int recv_buffer;
    while (scanf("%d", &send_buffer) != EOF) {
        sendto(
            sock,
            &send_buffer,
            sizeof(send_buffer),
            0,
            (struct sockaddr*)&destination,
            sizeof(destination));
        struct sockaddr source;
        recvfrom(
            sock,
            &recv_buffer,
            sizeof(recv_buffer),
            0,
            (struct sockaddr*)&source,
            NULL);
        printf("%d\n", recv_buffer);
    }
    exit(0);
}