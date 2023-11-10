#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

const int MAX_EVENTS = 100000;

volatile sig_atomic_t if_term = 0;

void handler_term(int signum)
{
    if_term = 1;
}

void add_to_epoll(int epoll_fd, int fdes)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLHUP;
    event.data.fd = fdes;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fdes, &event);
}

int read_and_send(int fd)
{
    char buffer[4096] = {};
    ssize_t size = 0;
    if (0 > (size = read(fd, buffer, sizeof(buffer)))) {
        return 1;
    }
    for (ssize_t idx = 0; idx < size; ++idx) {
        char upper_c = (char)toupper((unsigned char)buffer[idx]);
        write(fd, &upper_c, sizeof(upper_c));
    }
    return 0;
}

int main(int argc, char* argv[])
{
    struct sigaction sigact_term;
    memset(&sigact_term, 0, sizeof(sigact_term));
    sigact_term.sa_handler = handler_term;
    sigact_term.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGTERM, &sigact_term, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    int server;
    if (-1 == (server = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Can't create a socket");
        exit(1);
    }
    int flags = fcntl(server, F_GETFL);
    fcntl(server, F_SETFL, flags | O_NONBLOCK);
    in_port_t port = htons((short)strtol(argv[1], NULL, 10));
    struct in_addr ip;
    ip.s_addr = inet_addr("127.0.0.1");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = ip;
    if (-1 == bind(server, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("Bind failed");
        exit(1);
    }
    if (-1 == listen(server, SOMAXCONN)) {
        perror("Listen failed");
        exit(1);
    }

    int epoll_fd = epoll_create1(0);
    add_to_epoll(epoll_fd, server);

    struct epoll_event events[MAX_EVENTS];

    int clients[MAX_EVENTS];
    size_t clients_size = 0;

    while (1) {
        int events_pending = epoll_wait(
            epoll_fd, events, sizeof(events) / sizeof(*events), 1000);

        for (int i = 0; i < events_pending; ++i) {
            const struct epoll_event event = events[i];
            int readyToRead = event.events & EPOLLIN;
            int disconnected = event.events & EPOLLHUP;
            int errorOccured = event.events & EPOLLERR;
            int fdes = event.data.fd;
            if (readyToRead) {
                if (fdes == server) {
                    int client;
                    if (-1 == (client = accept(server, NULL, NULL))) {
                        perror("Connection failed");
                        continue;
                    }
                    clients[clients_size] = client;
                    ++clients_size;
                    add_to_epoll(epoll_fd, client);
                    int cl_flags = fcntl(client, F_GETFL);
                    fcntl(client, F_SETFL, cl_flags | O_NONBLOCK);
                } else {
                    if (1 == read_and_send(fdes)) {
                        disconnected = 1;
                    }
                }
            }
            if (disconnected || errorOccured) {
                shutdown(fdes, SHUT_RDWR);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fdes, NULL);
                close(fdes);
            }
        }
        if (if_term) {
            for (size_t i = 0; i < clients_size; ++i) {
                shutdown(clients[i], SHUT_RDWR);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, clients[i], NULL);
                close(clients[i]);
            }
            shutdown(server, SHUT_RDWR);
            close(server);
            close(epoll_fd);
            break;
        }
    }
    exit(0);
}
