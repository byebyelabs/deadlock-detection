#include "internal/detector.h"
#include "internal/utils.h"

static node_t* GLOBAL_LOCK_ORDERS = NULL;

void before_lock(pthread_mutex_t *m) {
    // get lock node if it exists
    node_t* curr = GLOBAL_LOCK_ORDERS;
    while (curr != NULL && curr->lock_number != m) {
        curr = curr->next;
    }

    // insert new node if not found
    if (curr == NULL) {
        // TODO: re-think about free later
        curr = malloc(sizeof(node_t*));
        curr->next = GLOBAL_LOCK_ORDERS;
        curr->lock_number = m;
        curr->avoid_lock_numbers = malloc(sizeof(pthread_mutex_t*) * MAX_LOCK_COUNT);

        GLOBAL_LOCK_ORDERS = curr;
    }
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
