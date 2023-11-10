#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

int MAX_EVENTS = 10000;

void makeNonBlocking(size_t N, int* in)
{
    for (size_t idx = 0; idx < N; ++idx) {
        int flags = fcntl(in[idx], F_GETFL);
        fcntl(in[idx], F_SETFL, flags | O_NONBLOCK);
    }
}

int addToEpoll(int epoll_fd, int fdes)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fdes;
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fdes, &event)) {
        return -1;
    }
    return 0;
}

void deleteFromEpoll(int epoll_fd, int fdes)
{

}

int readData(int fd, size_t* count)
{
    char buffer[4096] = {};
    size_t fd_count = read(fd, buffer, sizeof(buffer));
    if (fd_count == 0) {
        return 1;
    } else {
        *count += fd_count;
    }
    return 0;
}

extern size_t read_data_and_count(size_t N, int in[N])
{
    makeNonBlocking(N, in);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Epoll");
        return -1;
    }

    for (size_t idx = 0; idx < N; ++idx) {
        if (-1 == addToEpoll(epoll_fd, in[idx])) {
            perror("Failed to add to epoll");
            return -1;
        }
    }

    size_t count = 0;
    size_t opened_fds = N;
    struct epoll_event events[MAX_EVENTS];
    while (opened_fds != 0) {
        int read_available = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (size_t idx = 0; idx < read_available; ++idx) {
            if (events[idx].events & EPOLLIN) {
                if (1 == readData(events[idx].data.fd, &count)) {
                    close(events[idx].data.fd);
                    epoll_ctl(
                        epoll_fd, EPOLL_CTL_DEL, events[idx].data.fd, NULL);
                    --opened_fds;
                }
            }
        }
    }

    close(epoll_fd);
    return count;
}