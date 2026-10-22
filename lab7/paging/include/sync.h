#ifndef SYNC_H
#define SYNC_H
#include"os_type.h"
#include "list.h"
class spinlock{
    private:
    int lock_value;
    public:
    spinlock();
    void initialize();
    void lock();
    void unlock();


};

class Semaphore
{
private:
    uint32 counter; //临界资源个数
    List waiting;
    spinlock semLock;//保护counter

public:
    Semaphore();
    void initialize(uint32 counter);
    void P();
    void V();
};








#endif