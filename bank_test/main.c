#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bank.h"

#define STARTING_BALANCE 100
#define TRANSACTIONS 1000

void* run_transactions(void* arg) {
  // Get the account ID from the argument
  size_t account_id = *(size_t*)arg;

  // Run random transactions
  for (size_t i = 0; i < TRANSACTIONS; i++) {
    // Get this account's balance
    int balance = get_balance(account_id);

    // Is the account balance zero or negative?
    if (balance <= 0) {
      // Yes. Pause briefly and try a transaction on the next iteration
      // Hopefully someone has sent us funds by then
      usleep(100);

    } else {
      // Pick a random account to transfer to
      size_t other_account_id;
      do {
        other_account_id = rand() % NUM_ACCOUNTS;
      } while (other_account_id == account_id);

      // Generate a random amount to transfer
      int amount = rand() % balance;

      // Do the transfer
      transfer(account_id, other_account_id, amount);
    }
  }

  return NULL;
}

int main(int argc, char** argv) {
  // Seed the random number generator
  srand(time(NULL));

  // Initialize the bank
  init_bank();

  // Start each account off with a balance of STARTING_BALANCE
  for (size_t i = 0; i < NUM_ACCOUNTS; i++) {
    deposit(i, STARTING_BALANCE);
  }

  pthread_t threads[NUM_ACCOUNTS];
  size_t account_ids[NUM_ACCOUNTS];

  // Create threads to run transactions on each account
  for (size_t i = 0; i < NUM_ACCOUNTS; i++) {
    account_ids[i] = i;
    if (pthread_create(&threads[i], NULL, run_transactions, &account_ids[i])) {
      perror("pthread_create failed");
      exit(2);
    }
  }

  // Wait for each thread to finish
  for (size_t i = 0; i < NUM_ACCOUNTS; i++) {
    if (pthread_join(threads[i], NULL)) {
      perror("pthread_join failed");
      exit(2);
    }
  }

  // Show final balances
  printf("Final balances:\n");
  int total_funds = 0;
  for (size_t i = 0; i < NUM_ACCOUNTS; i++) {
    int balance = get_balance(i);
    printf("  account %lu: %d\n", i, balance);
    total_funds += balance;
  }

  // Does the total balance match the total amount deposited?
  if (total_funds == NUM_ACCOUNTS * STARTING_BALANCE) {
    printf("The total balance is correct.\n");

  } else if (total_funds < NUM_ACCOUNTS * STARTING_BALANCE) {
    printf("The total balance is wrong! The bank has misplaced %d in funds.\n",
           NUM_ACCOUNTS * STARTING_BALANCE - total_funds);

  } else {
    printf("The total balance is wrong! The bank has somehow gained %d in funds.\n",
           total_funds - NUM_ACCOUNTS * STARTING_BALANCE);
  }

  return 0;
}
