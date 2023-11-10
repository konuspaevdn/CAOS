#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct thread_info {
    size_t iter;
    size_t number;
    size_t index;
    double* arr;
    pthread_t thread;
    pthread_mutex_t* mutex;
};

void* thread_function(void* arg)
{
    struct thread_info* info = arg;
    size_t iter = info->iter;
    size_t number = info->number;
    size_t index = info->index;
    double* arr = info->arr;
    pthread_mutex_t* mutex = info->mutex;
    for (size_t i = 0; i < iter; ++i) {
        pthread_mutex_lock(&mutex[index]);
        arr[index] += 1;
        pthread_mutex_unlock(&mutex[index]);
        index = (index + number - 1) % number;
        pthread_mutex_lock(&mutex[index]);
        arr[index] += 0.99;
        pthread_mutex_unlock(&mutex[index]);
        index = (index + 2) % number;
        pthread_mutex_lock(&mutex[index]);
        arr[index] += 1.01;
        pthread_mutex_unlock(&mutex[index]);
    }
}

int main(int argc, char* argv[])
{
    const size_t N = (size_t)strtol(argv[1], NULL, 10);
    const size_t k = (size_t)strtol(argv[2], NULL, 10);

    double arr[k];
    memset(arr, 0, sizeof(arr));

    pthread_mutex_t mutexes[k];
    for (size_t i = 0; i < k; ++i) {
        pthread_mutex_init(&mutexes[i], NULL);
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    struct thread_info threads[k];
    for (size_t i = 0; i < k; ++i) {
        threads[i].iter = N;
        threads[i].number = k;
        threads[i].index = i;
        threads[i].arr = arr;
        threads[i].mutex = mutexes;
        if (0 !=
            pthread_create(
                &threads[i].thread, &attr, &thread_function, &threads[i])) {
            perror("Thread creating failed");
            pthread_attr_destroy(&attr);
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

    for (size_t i = 0; i < k; ++i) {
        pthread_join(threads[i].thread, NULL);
    }

    for (size_t i = 0; i < k; ++i) {
        pthread_mutex_destroy(&mutexes[i]);
    }

    for (size_t i = 0; i < k; ++i) {
        printf("%.10g\n", arr[i]);
    }

    exit(0);
}