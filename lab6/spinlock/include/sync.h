#ifndef SYNC_H
#define SYNC_H
#include"os_type.h"

class spinlock{
    private:
    int lock_value;
    public:
    spinlock();
    void initialize();
    void lock();
    void unlock();


};









#endif