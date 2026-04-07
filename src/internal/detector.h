// Bookkeeping to detect deadlocks

#include <pthread.h>

void before_lock(pthread_mutex_t *m);

void after_lock(pthread_mutex_t *m);

void before_unlock(pthread_mutex_t *m);

void after_unlock(pthread_mutex_t *m);
