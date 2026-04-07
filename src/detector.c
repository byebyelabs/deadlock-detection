#include "internal/detector.h"
#include "internal/utils.h"


void before_lock(pthread_mutex_t *m) {
    return;
}

void after_lock(pthread_mutex_t *m) {
    return;
}

void before_unlock(pthread_mutex_t *m) {
    return;
}

void after_unlock(pthread_mutex_t *m) {
    return;
}
