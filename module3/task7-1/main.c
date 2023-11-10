#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

volatile sig_atomic_t if_alarm = 0;

void handler(int signum)
{
    if_alarm = 1;
}

struct __attribute__((__packed__)) icmp_header {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;
    u_int16_t identifier;
    u_int16_t seq_num;
    char data[48];
};

uint16_t checksum(int count, void* addr)
{
    int result = 0;
    uint16_t* token = addr;

    while (count > 1) {
        result += *token++;
        count -= 2;
    }

    if (count == 1) {
        result += *(unsigned char*)token;
    }

    result = (result >> 16) + (result & 0xffff);
    result = ~(result + (result >> 16));

    return (uint16_t)result;
}

int main(int argc, char* argv[])
{
    struct sigaction sigact_alrm;
    memset(&sigact_alrm, 0, sizeof(sigact_alrm));
    sigact_alrm.sa_handler = handler;
    sigact_alrm.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGALRM, &sigact_alrm, NULL))
    {
        perror("Sigaction failed");
        exit(1);
    }

    if (argc < 4) {
        perror("Not enough arguments");
        exit(1);
    }

    int sock;
    if (-1 == (sock = socket(AF_INET, SOCK_RAW, 1))) {
        perror("Socket creation failed");
        exit(1);
    }

    uint timeout = strtol(argv[2], NULL, 10);
    unsigned long interval = strtol(argv[3], NULL, 10);

    srandom(time(NULL));

    int counter = 0;
    int iter = 0;

    int id = (int)random();
    alarm(timeout);

    while (!if_alarm) {
        char send[sizeof(struct icmp_header)] = {};
        struct icmp_header* icmp_header = (struct icmp_header*)send;
        icmp_header->type = 8;
        icmp_header->code = 0;
        icmp_header->checksum = 0;
        icmp_header->identifier = id;
        icmp_header->seq_num = iter++;
        icmp_header->checksum = checksum(sizeof(struct icmp_header), icmp_header);
        struct in_addr ip;
        ip.s_addr = inet_addr(argv[1]);
        struct sockaddr_in dest;
        memset(&dest, 0, sizeof(dest));
        dest.sin_family = AF_INET;
        dest.sin_addr = ip;
        if (-1 == sendto(
            sock, send, sizeof(send), 0, (struct sockaddr*)&dest, sizeof(dest))) {
            perror("Ping failed");
            continue;
        }
        char recv[200];
        recvfrom(sock, recv, sizeof(recv), 0, NULL, NULL);
        icmp_header = (struct icmp_header*)(recv + 20);
        u_int16_t chksum = icmp_header->checksum;
        icmp_header->checksum = 0;
        if (checksum(sizeof(struct icmp_header), icmp_header) == chksum) {
            ++counter;
        }
        usleep(interval);
    }

    printf("%d\n", counter);
    close(sock);
    exit(0);
}