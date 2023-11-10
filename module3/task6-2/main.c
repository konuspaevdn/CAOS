#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Item {
    struct Item* next;
    int64_t value;
} item_t;

typedef struct Info {
    size_t num;
    size_t iter;
    item_t* items;
} info_t;

static item_t* front = NULL;

void* thread_function(void* arg)
{
    info_t* info = arg;
    size_t num = info->num;
    size_t iter = info->iter;
    item_t* items = info->items;
    for (size_t i = num * iter; i < (num + 1) * iter; ++i) {
        items[i].value = (int64_t)i;
        item_t* pr_head = atomic_exchange(&front, &items[i]);
        items[i].next = pr_head;
        sched_yield();
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    const size_t N = (size_t)strtol(argv[1], NULL, 10);
    const size_t k = (size_t)strtol(argv[2], NULL, 10);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

    item_t items[N * k];
    pthread_t threads[N];
    info_t info[N];

    for (size_t i = 0; i < N; ++i) {
        info[i].num = i;
        info[i].iter = k;
        info[i].items = items;
        if (0 !=
            pthread_create(&threads[i], &attr, &thread_function, &info[i])) {
            perror("Thread creation failed");
            pthread_attr_destroy(&attr);
            exit(1);
        }
    }
    pthread_attr_destroy(&attr);

    for (size_t i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL);
    }

    item_t* ptr = front;
    while (ptr != NULL) {
        printf("%" PRIi64 "\n", ptr->value);
        ptr = ptr->next;
    }

    exit(0);
}