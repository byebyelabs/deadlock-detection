#include "internal/detector.h"
#include "internal/rlocks.h"
#include "internal/utils.h"

#include <execinfo.h>

static node_t *GLOBAL_LOCK_ORDERS = NULL;
static pthread_mutex_t DETECTOR_LOCK = PTHREAD_MUTEX_INITIALIZER;

static _Thread_local int NUM_TRD_LCL_CURR_HELD_LOCKS = 0;
static _Thread_local pthread_mutex_t *TRD_LCL_CURR_HELD_LOCKS[MAX_LOCK_COUNT];

void print_error_trace() {
  const int MAX_BACKTRACE_SIZE = 15;
  void *buffer[MAX_BACKTRACE_SIZE];
  int n_ptrs = backtrace(buffer, MAX_BACKTRACE_SIZE);
  fprintf(stderr, "WARNING: Potential Deadlock Detected\n");
  fprintf(stderr, "Backtrace was %d addresses long\n", n_ptrs);
  char **bt_syms = backtrace_symbols(buffer, n_ptrs);
  if (bt_syms == NULL) {
    perror("Could not get backtrace symbols!");
  } else {
    for (size_t j = 0; j < n_ptrs; j++) {
      fprintf(stderr, "%s\n", bt_syms[j]);
    }
  }
}

void verify_no_deadlock(pthread_mutex_t *m) {
  // get lock node if it exists
  node_t *curr = GLOBAL_LOCK_ORDERS;
  while (curr != NULL && curr->lock_number != m) {
    curr = curr->next;
  }

  // insert new node if not found
  if (curr == NULL) {
    // TODO: re-think about free later
    curr = malloc(sizeof(node_t));
    curr->next = GLOBAL_LOCK_ORDERS;
    curr->lock_number = m;
    curr->avoid_lock_numbers =
        calloc(MAX_LOCK_COUNT, sizeof(pthread_mutex_t *));
    curr->avoid_lock_numbers[0] = NULL; // NULL-terminating list

    // update global head to curr (🙄)
    GLOBAL_LOCK_ORDERS = curr;
  }

  // assert m is not in any held lock's avoid_lock_numbers
  for (int i = 0; i < NUM_TRD_LCL_CURR_HELD_LOCKS; i++) {
    int found_mutex_in_avoid_list = 0;
    int checked = 0;

    // find the node for the held lock
    node_t *held_node = GLOBAL_LOCK_ORDERS;
    while (held_node != NULL &&
           held_node->lock_number != TRD_LCL_CURR_HELD_LOCKS[i]) {
      held_node = held_node->next;
    }
    if (held_node == NULL) {
      // TODO: decide if we throw error here or continue
      perror("ERROR: Some Held Lock's Node Not Found");
      exit(EXIT_FAILURE);
    }

    // check if m exists in any held lock's avoid lock numbers
    while (!found_mutex_in_avoid_list && checked < MAX_LOCK_COUNT &&
           held_node->avoid_lock_numbers[checked] != NULL) {
      found_mutex_in_avoid_list = (held_node->avoid_lock_numbers[checked] == m);
      checked++;
    }

    // found a potential deadlock condition!
    if (found_mutex_in_avoid_list) {
      print_error_trace();
      exit(EXIT_FAILURE);
    }
  }
}

void before_lock(pthread_mutex_t *m) {
  real_pthread_mutex_lock(&DETECTOR_LOCK);
  verify_no_deadlock(m);
  real_pthread_mutex_unlock(&DETECTOR_LOCK);
  return;
}

void after_lock(pthread_mutex_t *m) {
  real_pthread_mutex_lock(&DETECTOR_LOCK);

  // ensure no deadlock's found
  verify_no_deadlock(m);

  // search for m node (must exist!)
  node_t *curr = GLOBAL_LOCK_ORDERS;
  while (curr != NULL && curr->lock_number != m) {
    curr = curr->next;
  }

  if (curr == NULL) {
    // TODO: decide if we throw error here or continue
    perror("ERROR: New Lock's Node Not Found");
    exit(EXIT_FAILURE);
  }

  // now, add all currently held locks to this newly acquired lock's avoid list
  for (int i = 0; i < NUM_TRD_LCL_CURR_HELD_LOCKS; i++) {
    // get this held lock's node struct
    node_t *held_node = GLOBAL_LOCK_ORDERS;
    while (held_node != NULL &&
           held_node->lock_number != TRD_LCL_CURR_HELD_LOCKS[i]) {
      held_node = held_node->next;
    }

    if (held_node == NULL) {
      // TODO: decide if we throw error here or continue
      perror("ERROR: Some Held Lock's Node Not Found");
      exit(EXIT_FAILURE);
    }

    // check if held node is already in the current node's avoid list
    int index = 0;
    int found_held_node = 0;
    while (!found_held_node && index < MAX_LOCK_COUNT &&
           curr->avoid_lock_numbers[index] != NULL) {
      found_held_node =
          (held_node->lock_number == curr->avoid_lock_numbers[index]);
      index++;
    }

    if (index == MAX_LOCK_COUNT) {
      perror("ERROR: Not Enough Space, Increase `MAX_LOCK_COUNT`");
      exit(EXIT_FAILURE);
    }

    // if not found, save the unique value to list!
    curr->avoid_lock_numbers[index] = TRD_LCL_CURR_HELD_LOCKS[i];
    if (++index < MAX_LOCK_COUNT) {
      curr->avoid_lock_numbers[index] = NULL;
    }
  }

  // save the newly held lock to the thread's lock set
  TRD_LCL_CURR_HELD_LOCKS[NUM_TRD_LCL_CURR_HELD_LOCKS] = m;
  NUM_TRD_LCL_CURR_HELD_LOCKS += 1;

  real_pthread_mutex_unlock(&DETECTOR_LOCK);
  return;
}

void before_unlock(pthread_mutex_t *m) { return; }

void after_unlock(pthread_mutex_t *m) { return; }
