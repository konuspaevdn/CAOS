#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct thread_info {
    uint64_t left_border;
    uint64_t right_border;
    size_t max_count;
    pthread_mutex_t* mutex1;
    pthread_mutex_t* mutex2;
    pthread_cond_t* pass_done;
    pthread_cond_t* generation_done;
    uint64_t buffer;
    int right_border_reached;
    int signal_generation;
    int signal_pass;
};

int prime(uint64_t num)
{
    for (uint64_t i = 2; i < num; ++i) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

void* thread_function(void* arg)
{
    struct thread_info* info = arg;
    uint64_t left_border = info->left_border;
    uint64_t right_border = info->right_border;
    size_t max_count = info->max_count;
    pthread_mutex_t* mutex1 = info->mutex1;
    pthread_mutex_t* mutex2 = info->mutex2;
    pthread_cond_t* pass_done = info->pass_done;
    pthread_cond_t* generation_done = info->generation_done;

    pthread_mutex_lock(mutex2);
    size_t counter = 0;

    for (uint64_t num = 2; num <= right_border; ++num) {
        if (left_border <= num && prime(num)) {
            info->buffer = num;
            pthread_mutex_lock(mutex1);
            pthread_mutex_unlock(mutex1);
            info->signal_generation = 1;
            pthread_cond_signal(generation_done);
            while (!info->signal_pass) {
            	pthread_cond_wait(pass_done, mutex2);
            }
            info->signal_pass = 0;
            ++counter;
            if (counter == max_count) {
                pthread_mutex_unlock(mutex2);
                pthread_exit(NULL);
            }
        }
    }
    pthread_mutex_unlock(mutex2);
    info->right_border_reached = 1;
    pthread_cond_signal(generation_done);
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    const uint64_t A = (uint64_t)strtol(argv[1], NULL, 10);
    const uint64_t B = (uint64_t)strtol(argv[2], NULL, 10);
    const size_t N = (size_t)strtol(argv[3], NULL, 10);

    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t pass_done = PTHREAD_COND_INITIALIZER;
    pthread_cond_t generation_done = PTHREAD_COND_INITIALIZER;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    struct thread_info info;
    info.left_border = A;
    info.right_border = B;
    info.max_count = N;
    info.mutex1 = &mutex1;
    info.mutex2 = &mutex2;
    info.pass_done = &pass_done;
    info.generation_done = &generation_done;
    info.right_border_reached = 0;
    info.signal_generation = 0;
    info.signal_pass = 0;

    pthread_mutex_lock(&mutex1);

    pthread_t thread;
    if (0 != pthread_create(&thread, &attr, &thread_function, &info)) {
        perror("Thread creation failed");
        pthread_mutex_destroy(&mutex1);
        pthread_mutex_destroy(&mutex2);
        pthread_cond_destroy(&pass_done);
        pthread_cond_destroy(&generation_done);
        pthread_attr_destroy(&attr);
        exit(1);
    }
    pthread_attr_destroy(&attr);

    for (size_t i = 0; i < N; ++i) {
        if (info.right_border_reached) {
            break;
        }
        while (!info.signal_generation) {
        	pthread_cond_wait(&generation_done, &mutex1);
        }
        info.signal_generation = 0;
        if (info.right_border_reached) {
            break;
        }
        uint64_t prime_num = info.buffer;
        pthread_mutex_lock(&mutex2);
        pthread_mutex_unlock(&mutex2);
        info.signal_pass = 1;
        pthread_cond_signal(&pass_done);
        printf("%" PRIu64 "\n", prime_num);
    }

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&pass_done);
    pthread_cond_destroy(&generation_done);
    exit(0);
}
