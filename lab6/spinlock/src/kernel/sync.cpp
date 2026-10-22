#include "sync.h"
#include "asm_utils.h"

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