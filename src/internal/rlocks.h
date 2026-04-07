// Real locking and unlocking functions from <pthread.h>

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

typedef int (*mutex_fn_t)(pthread_mutex_t *);

int real_pthread_mutex_lock(pthread_mutex_t *m);

int real_pthread_mutex_unlock(pthread_mutex_t *m);
