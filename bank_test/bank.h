#pragma once

#include <stdlib.h>

// How many bank accounts are there?
#define NUM_ACCOUNTS 10

// Special constant for an invalid account
#define INVALID_ACCOUNT -999999

// Initialize all of the accounts and any global state at this bank
void init_bank();

// Get the balance for a bank account
int get_balance(size_t account_id);

// Deposit funds to an account from an outside source
void deposit(size_t account_id, int amount);

// Transfer funds from one account to another
void transfer(size_t from_id, size_t to_id, int amount);
