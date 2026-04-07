#include "internal/rlocks.h"

// Citation: https://arc.net/l/quote/tmdpqzak
int real_pthread_mutex_lock(pthread_mutex_t *m) {
  static mutex_fn_t real_fn = NULL;
  if (real_fn == NULL) {
    real_fn = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    if (real_fn == NULL) {
      fprintf(stderr, "Failed to locate pthread_mutex_lock: %s\n", dlerror());
    }
  }

  return real_fn(m);
}

int real_pthread_mutex_unlock(pthread_mutex_t *m) {
  static mutex_fn_t real_fn = NULL;
  if (real_fn == NULL) {
    real_fn = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    if (real_fn == NULL) {
      fprintf(stderr, "Failed to locate pthread_mutex_unlock: %s\n", dlerror());
    }
  }

  return real_fn(m);
}
