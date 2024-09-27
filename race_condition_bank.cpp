#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

class BankAccount {
private:
    int balance;
    std::mutex mtx;
public:
    BankAccount(int initialBalance) : balance(initialBalance) {}

    void deposit(int amount, const std::string& threadName, bool sync) {
        
        if(sync) mtx.lock(); // Lock for race condition
        int currentBalance = balance;
        balance = currentBalance + amount;
        std::cout<<"Thread: "<<threadName
            <<" Deposit: "<<amount<<" Balance: "<<balance<<std::endl;
        if(sync) mtx.unlock(); // unlock for race condition
    }
    void withdraw(int amount, const std::string& threadName, bool sync) {
        if(sync) mtx.lock(); // Lock for race condition
        int currentBalance = balance;
        balance = currentBalance - amount;
         std::cout<<"Thread: "<<threadName
            <<" Withdraw: "<<amount<<" Balance: "<<balance<<std::endl;
        if(sync) mtx.unlock(); // unlock for race condition
    }
    int getBalance() const {
        return balance;
    }
};
////////////// CRITICAL SECTION //////////////////////////////////////////
void performTransactions(BankAccount &account, const std::string& threadName, bool sync) {
    for (int i = 0; i < 1000; ++i) {
        account.deposit(5, threadName, sync);   // Deposita 1
        account.withdraw(5, threadName, sync);  // Preleva 1
    }
}
//////////////////////////////////////////////////////////////////////////
int main() {
    std::cout<<"---Race Condition Dimostration---"<<std::endl;
    std::cout<<"-----Bank Account Simulation-----"<<std::endl;
    BankAccount account(100); // Crea un conto con un saldo iniziale di 100
    std::cout<<"\nBank Account Balance: "<<account.getBalance()<<std::endl;
    std::cout<<std::endl;
    // Choose syncronization
    char input;
    bool sync=false;
    do{    
        std::cout<<"With syncronization?: (y)es (n)o: "<<input;
        std::cin>>input;
    }while(input!='y' && input!='n');
    if(input=='y') sync=true;
    std::thread thread1(performTransactions, std::ref(account), "1", sync);
    std::thread thread2(performTransactions, std::ref(account), "2", sync);
    // Main Thread waits for thread1 and thread2
    thread1.join();
    thread2.join();
    std::cout << "Final Balance: " << account.getBalance() << std::endl;
    return 0;
}
