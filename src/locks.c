#include "internal/rlocks.h"
#include "internal/detector.h"

int pthread_mutex_lock(pthread_mutex_t *m) {
    // A list of locks currently held in this thread
    

    before_lock(m);
    int result = real_pthread_mutex_lock(m);
    after_lock(m);

    return result;
}

int pthread_mutex_unlock(pthread_mutex_t *m) {
    before_unlock(m);
    int result = real_pthread_mutex_unlock(m);
    after_unlock(m);

    return result;
}