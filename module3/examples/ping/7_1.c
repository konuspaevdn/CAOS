#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

const int data_size = 48;
const int ip_head_size = 20;
const int max_size = 4096;

uint64_t cur_time() {
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec;
}

typedef struct __attribute__((__packed__)) request{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq_num;
	char data[48];

} request_t;

typedef struct{
	unsigned char type;
	unsigned char code;
	unsigned  short checksum;
	unsigned  short id;
	unsigned  short seq_num;
} reply_t;

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

int main(int argc, char* argv[]) {

	char* ipv4_addr = argv[1];
	int timeout = atoi(argv[2]);
	int interval = atoi(argv[3]);
	int sock = socket(AF_INET, SOCK_RAW, 1);
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
    	.sin_addr = inet_addr(ipv4_addr)
	};
	uint64_t start_time = cur_time();
	uint64_t count = 0;
	char send_buf[sizeof(request_t)];
	char recv_buf[max_size];
	int i = 0;

	while((cur_time() - start_time) < timeout) {
		request_t ping_pkt;
		ping_pkt.type = 8;
		ping_pkt.code = 0;
		ping_pkt.checksum = 0;
		ping_pkt.id = rand();
		ping_pkt.seq_num = i;
		ping_pkt.checksum = checksum(sizeof(request_t), &ping_pkt);
		int rand_id = ping_pkt.id;
		int last_checksum = ping_pkt.checksum;
		memcpy(send_buf, &ping_pkt, sizeof(request_t));
		sendto(sock, send_buf, sizeof(request_t), 0, (struct sockaddr *)&addr, sizeof(addr));
		recvfrom(sock, recv_buf, sizeof(request_t) + ip_head_size, 0, NULL, NULL);
		request_t recv;
		char* icmp_buf = NULL;
		icmp_buf = recv_buf + ip_head_size;
		memcpy(&recv, icmp_buf, sizeof(recv));
		if (recv.id == ping_pkt.id) {
			++count;
		}
		++i;
		usleep(interval);
	
	}
	printf("%lu\n", count);
}
