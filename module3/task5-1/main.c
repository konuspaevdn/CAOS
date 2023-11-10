#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_function(void* arg)
{
    int64_t part_sum = 0;
    int64_t value = 0;
    while (EOF != scanf("%"SCNd64, &value))
    {
        part_sum += value;
    }
    void* retval = (void*)part_sum;
    return retval;
}

int main(int argc, char* argv[])
{
    int N = (int)strtol(argv[1], NULL, 10);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    pthread_t threads[N];
    int64_t sum = 0;

    for (int i = 0; i < N; ++i)
    {
        if (0 != pthread_create(&threads[i], &attr, &thread_function, NULL))
        {
            perror("Thread creating failed");
            pthread_attr_destroy(&attr);
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

    for (int i = 0; i < N; ++i)
    {
        int64_t part_sum = 0;
        int64_t* part_sum_ptr = &part_sum;
        pthread_join(threads[i], (void**)part_sum_ptr);
        sum += part_sum;
    }

    printf("%"PRId64"\n", sum);
    exit(0);
}