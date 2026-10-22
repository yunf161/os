#include "sync.h"
#include "asm_utils.h"
#include "os_modules.h"

spinlock:: spinlock(){
    initialize();
}
    void spinlock::initialize(){
        lock_value=0;
    }
    void spinlock::lock(){
      
        int key=1;
        do{
            asm_atomic_exchange(&key,& lock_value);
        }while(key==1);
    }
    void spinlock::unlock(){
        lock_value=0;
    }

   Semaphore:: Semaphore(){
      initialize(1);
   }
    void Semaphore:: initialize(uint32 counter){
        this->counter=counter;
        waiting.initialize();
        semLock.initialize();
    }
    void Semaphore:: P(){
       PCB *cur = nullptr;

    while (true)
    {
        semLock.lock();
        if (counter > 0)
        {
            --counter;
            semLock.unlock();
            return;
        }

        cur = programManager.running;
        waiting.push_back(&(cur->tagInGeneralList));
        cur->status = ProgramStatus::BLOCKED;

        semLock.unlock();
        programManager.schedule();
    }
    }
    void Semaphore::V()
    {
    semLock.lock();
    ++counter;
    if (waiting.size())
    {
        PCB *program = ListItem2PCB(waiting.front(), tagInGeneralList);
        waiting.pop_front();
        semLock.unlock();
        programManager.MESA_WakeUp(program);
    }
    else
    {
        semLock.unlock();
    }
}