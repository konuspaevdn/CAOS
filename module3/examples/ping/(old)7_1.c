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

typedef struct{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t seq_num;

} request_t;

typedef struct{
	unsigned char type;
	unsigned char code;
	unsigned  short checksum;
	unsigned  short id;
	unsigned  short seq_num;
} reply_t;

unsigned short checksum(uint64_t count,request_t* ping){
    register unsigned short sum = 0;
    unsigned short* addr = (unsigned short*)ping;
    while( count > 1 )  {
           //  This is the inner loop 
        sum += *addr++;
        count -= 2;
    }

           // Add left-over byte, if any 
    if( count > 0 )
        sum += * (unsigned char *) addr;

           //  Fold 32-bit sum to 16 bits 
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

int main(int argc, char* argv[]) {

	char* ipv4_addr = argv[1];
	int timeout = atoi(argv[2]);
	int interval = atoi(argv[3]);
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
    	.sin_addr = inet_addr(ipv4_addr)
	};
	uint64_t start_time = cur_time();
	uint64_t count = 0;
	char send_buf[sizeof(request_t) + data_size];
	char recv_buf[max_size];

	while((cur_time() - start_time) < timeout) {
		request_t ping_pkt;
		ping_pkt.type = 8;
		ping_pkt.code = 0;
		ping_pkt.checksum = 0;
		ping_pkt.id = rand();
		ping_pkt.seq_num = 1;
		ping_pkt.checksum = checksum(sizeof(request_t), &ping_pkt);
		int rand_id = ping_pkt.id;
		int last_checksum = ping_pkt.checksum;
		memcpy(send_buf, &ping_pkt, sizeof(request_t));
		memset(send_buf + sizeof(request_t), 'x', 48);
		sendto(sock, send_buf, sizeof(request_t) + data_size, 0, (struct sockaddr *)&addr, sizeof(addr));
		recvfrom(sock, recv_buf, sizeof(request_t) + data_size, 0, NULL, NULL);
		request_t recv;
		char* icmp_buf = NULL;
		icmp_buf = recv_buf + ip_head_size;
		memcpy(&recv, icmp_buf, sizeof(recv));
		if (recv.id == ping_pkt.id) {
			++count;
		}
		/*printf("%d\n", recv->type);
		printf("%d\n", recv->code);
		printf("%d\n", recv->id);
		printf("%d\n", recv->seq_num);
		break;*/
		usleep(interval);
	
	}
	printf("%lu\n", count);
}