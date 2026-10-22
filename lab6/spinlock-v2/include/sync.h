#ifndef SYNC_H
#define SYNC_H
#include"os_type.h"

class spinlock{
    private:
      volatile int lock_value;
    public:
    spinlock();
    void initialize();
    void lock(); 
    void unlock();


};









#endif