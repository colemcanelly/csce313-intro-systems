#include "BankAccount.h"

// interface function to perform *threadsafe* transactions
// "you will need to implement the transaction_threadsafe() function...
// "using mutexes to ensure proper concurrency"
void BankAccount::do_transaction_threadsafe(int _amount) {
    // TODO: LOCK other threads from using transaction() at the same time
    m.lock();

    do_transaction(_amount);
    // TODO: UNLOCK for the next thread to access transaction()
    m.unlock();
    
}

/***************** ALL FUNCTIONS BELOW THIS LINE ARE COMPLETE ****************/

// default constructor
// "assume the bank account starts with $0"
BankAccount::BankAccount() : balance(0) {}

// interface function to perform transactions
void BankAccount::do_transaction(int _amount) {
    balance += _amount;
}

// interface function to perform transactions
void BankAccount::transaction(int _amount, bool threadsafe) {
    //sleep to simulate some I/O delay (e.g. fetching account info)
    usleep(rand() % 100);

    if(threadsafe) do_transaction_threadsafe(_amount);
    else do_transaction(_amount);
}

// print function for consistency
void BankAccount::print_balance() {
    std::string currency = this->balance < 0 ? "-$" : "$";
    std::cout << currency << abs(this->balance);
}
