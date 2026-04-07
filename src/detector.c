#include "internal/detector.h"
#include "internal/rlocks.h"
#include "internal/utils.h"

static node_t *GLOBAL_LOCK_ORDERS = NULL;
static pthread_mutex_t DETECTOR_LOCK = PTHREAD_MUTEX_INITIALIZER;

static _Thread_local int NUM_TRD_LCL_CURR_HELD_LOCKS = 0;
static _Thread_local pthread_mutex_t *TRD_LCL_CURR_HELD_LOCKS[MAX_LOCK_COUNT];

void print_global_lock_order_ds() {
  return;
  if (GLOBAL_LOCK_ORDERS == NULL) {
    printf("(NULL)\n");
  }

  node_t *curr = GLOBAL_LOCK_ORDERS;
  while (curr != NULL) {
    printf("lock_num: %p\n", curr->lock_number);
    printf(" avoid_lock_nums:\n");

    int index = 0;
    while (curr->avoid_lock_numbers[index] != NULL) {
      printf("  %p\n", curr->avoid_lock_numbers[index]);
      index++;
    }

    curr = curr->next;
  }

  printf("TRD_LCL_CURR_HELD_LOCKS:\n");
  for (int i = 0; i < NUM_TRD_LCL_CURR_HELD_LOCKS; i++) {
    printf("  %p\n", TRD_LCL_CURR_HELD_LOCKS[i]);
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
      continue;
    }

    // check if m exists in any held lock's avoid lock numbers
    while (!found_mutex_in_avoid_list && checked < MAX_LOCK_COUNT &&
           held_node->avoid_lock_numbers[checked] != NULL) {
      found_mutex_in_avoid_list = (held_node->avoid_lock_numbers[checked] == m);
      checked++;
    }

    // found a potential deadlock condition!
    if (found_mutex_in_avoid_list) {
      perror("WARNING: Potential Deadlock Detected");
      exit(EXIT_FAILURE);
    }
  }
}

void before_lock(pthread_mutex_t *m) {
  printf("before lock started for %p\n", m);

  real_pthread_mutex_lock(&DETECTOR_LOCK);
  verify_no_deadlock(m);
  real_pthread_mutex_unlock(&DETECTOR_LOCK);

  print_global_lock_order_ds();
  printf("before lock ended for %p\n", m);

  return;
}

void after_lock(pthread_mutex_t *m) {
  printf("after lock started for %p\n", m);

  real_pthread_mutex_lock(&DETECTOR_LOCK);
  verify_no_deadlock(m);

  // search for m in GLOBAL_LOCK_ORDERS
  node_t *curr = GLOBAL_LOCK_ORDERS;
  while (curr != NULL && curr->lock_number != m) {
    curr = curr->next;
  }

  // union curr->avoid_lock_numbers and TRD_LCL_CURR_HELD_LOCKS[i], save to
  // curr->avoid_lock_numbers go through all held locks in thread, and then add
  // them to newest lock's avoid_lock_numbers
  int index = 0;
  while (index < MAX_LOCK_COUNT && curr->avoid_lock_numbers[index] != NULL)
    index++;

  if (index >= MAX_LOCK_COUNT - NUM_TRD_LCL_CURR_HELD_LOCKS) {
    perror("Not enough space, increace `MAX_LOCK_COUNT`");
    exit(EXIT_FAILURE);
  }

  for (int j = 0; j < NUM_TRD_LCL_CURR_HELD_LOCKS; j++) {
    curr->avoid_lock_numbers[index] = TRD_LCL_CURR_HELD_LOCKS[j];
    index++;
  }

  curr->avoid_lock_numbers[index] = NULL;

  if (NUM_TRD_LCL_CURR_HELD_LOCKS == MAX_LOCK_COUNT) {
    perror("Not enough space, increace `MAX_LOCK_COUNT`");
    exit(EXIT_FAILURE);
  }

  TRD_LCL_CURR_HELD_LOCKS[NUM_TRD_LCL_CURR_HELD_LOCKS] = m;
  NUM_TRD_LCL_CURR_HELD_LOCKS += 1;
  printf("new NUM_TRD_LCL_CURR_HELD_LOCKS: %d\n", NUM_TRD_LCL_CURR_HELD_LOCKS);

  print_global_lock_order_ds();
  printf("after lock ended for %p\n", m);

  real_pthread_mutex_unlock(&DETECTOR_LOCK);

  return;
}

void before_unlock(pthread_mutex_t *m) { return; }

void after_unlock(pthread_mutex_t *m) { return; }
