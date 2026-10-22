#include "sync.h"
#include "asm_utils.h"

spinlock:: spinlock(){
    initialize();
}
    void spinlock::initialize(){
        lock_value=0;
    }
    void spinlock::lock(){
      asm_get_lock(&lock_value);
    }
    void spinlock::unlock(){
            asm_release_lock(&lock_value);
    } 