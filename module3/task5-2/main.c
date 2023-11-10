#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sv[2];
int if_ending = 0;

void* thread_function(void* arg)
{
    int64_t number = (int64_t)arg;
    while (1) {
        char value_str[15] = {};
        recv(sv[number - 1], value_str, sizeof(value_str), 0);
        if (if_ending) {
            pthread_exit(NULL);
        }
        int value = (int)strtol(value_str, NULL, 10);
        switch (number) {
        case 1:
            value -= 3;
            break;
        case 2:
            value += 5;
            break;
        default:
            break;
        }
        printf("%d\n", value);
        if (0 == value || value > 100) {
            if_ending = 1;
            close(sv[0]);
            close(sv[1]);
            exit(0);
        }
        memset(value_str, 0, sizeof(value_str));
        snprintf(value_str, sizeof(value_str), "%d", value);
        send(
            sv[number - 1],
            value_str,
            strnlen(value_str, sizeof(value_str)),
            0);
    }
}

int main(int argc, char* argv[])
{
    int N = (int)strtol(argv[1], NULL, 10);
    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        perror("Couldn't create sockets");
        exit(1);
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

    pthread_t one;
    pthread_t two;

    if (0 != pthread_create(&one, &attr, &thread_function, (void*)1)) {
        perror("Thread creating failed");
        pthread_attr_destroy(&attr);
        exit(1);
    }
    if (0 != pthread_create(&two, &attr, &thread_function, (void*)2)) {
        perror("Thread creating failed");
        pthread_attr_destroy(&attr);
        exit(1);
    }

    char value[4] = {};
    snprintf(value, sizeof(value), "%d", N);
    send(sv[1], value, strnlen(value, sizeof(value)), 0);

    pthread_attr_destroy(&attr);
    pthread_exit(NULL);
}