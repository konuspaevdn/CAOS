#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t threads[1000];

void* thread_function(void* arg)
{
    int value;
    if (EOF != scanf("%d", &value)) {
        if (0 != pthread_create(
                     &threads[(size_t)arg + 1],
                     NULL,
                     &thread_function,
                     (void*)((size_t)arg + 1))) {
            perror("Thread creating failed");
            exit(1);
        }
        pthread_join(threads[(size_t)arg + 1], NULL);
        printf("%d\n", value);
    }
}

int main()
{
    if (0 != pthread_create(&threads[0], NULL, &thread_function, (void*)0)) {
        perror("Thread creating failed");
        exit(1);
    }
    pthread_join(threads[0], NULL);
    exit(0);
}