// Bookkeeping to detect deadlocks

#include <pthread.h>
#include <stdlib.h>
#define MAX_LOCK_COUNT 100

// global data structure to store state
typedef struct node {
    pthread_mutex_t* lock_number;
    pthread_mutex_t** avoid_lock_numbers;
    struct node* next;
} node_t;

void before_lock(pthread_mutex_t *m);

void after_lock(pthread_mutex_t *m);

void before_unlock(pthread_mutex_t *m);

void after_unlock(pthread_mutex_t *m);
