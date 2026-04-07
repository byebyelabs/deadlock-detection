#include "bank.h"

#include <pthread.h>

// Data associated with an account
typedef struct account {
  int balance;
  pthread_mutex_t lock;
} account_t;

// All of this bank's accounts
account_t accounts[NUM_ACCOUNTS];

// Initialize all of the accounts and any global state at this bank
void init_bank() {
  // Initialize each account
  for (size_t i = 0; i < NUM_ACCOUNTS; i++) {
    accounts[i].balance = 0;
    pthread_mutex_init(&accounts[i].lock, NULL);
  }
}

// Get the balance for a bank account
int get_balance(size_t account_id) {
  // Make sure the account id is valid
  if (account_id >= NUM_ACCOUNTS) return INVALID_ACCOUNT;

  // Return the balance
  return accounts[account_id].balance;
}

// Deposit funds to an account from an outside source
void deposit(size_t account_id, int amount) {
  // If the account id is invalid, return immediately
  if (account_id >= NUM_ACCOUNTS) return;

  // Add funds
  accounts[account_id].balance += amount;
}

// Transfer funds from one account to another
void transfer(size_t from_id, size_t to_id, int amount) {
  // If either account id is invalid, return immediately
  if (from_id >= NUM_ACCOUNTS || to_id >= NUM_ACCOUNTS) return;

  // If the from and to accounts are the same, return immediately
  if (from_id == to_id) return;

  // If the amount is zero or negative, return immediately
  if (amount <= 0) return;

  // lock accounts
  pthread_mutex_lock(&accounts[from_id].lock);
  pthread_mutex_lock(&accounts[to_id].lock);

  // Transfer funds
  accounts[from_id].balance = accounts[from_id].balance - amount;
  accounts[to_id].balance = accounts[to_id].balance + amount;

  // unlock accounts
  pthread_mutex_unlock(&accounts[to_id].lock);
  pthread_mutex_unlock(&accounts[from_id].lock);
}
