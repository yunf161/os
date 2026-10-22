
#include "philosopher.h"
#include "os_modules.h"
#include "asm_utils.h"

Philosopher::Philosopher() {
    initialize(0);
}

void Philosopher::initialize(int id) {
    this->id = id;
    this->leftFork = -1;
    this->rightFork = -1;
    this->state = THINKING;
}
 
int Philosopher::getId() {
    return id;
}

State Philosopher::getState() {
    return state;
}

void Philosopher::setState(State newState) {
    state = newState;
}
int Philosopher::getLeftForkid() {
    return leftFork;
}
int Philosopher::getRightForkid() {
    return rightFork;
}

int Philosopher::getLeftFork() {
    if(state!=State::HUNGRY){
        printf("ph %d is not hungry in getLeftFork",id);
        return -1;
    } 
    bool success=false;
    int leftid=id-1;
    if(leftid==0){leftid=5;}
    Semaphores[leftid].P();
    if (lockstate[leftid]==-1){
        lockstate[leftid]=id;
        success=true;
    }
    if(success){
        leftFork=leftid;
        return leftFork;
    }

    return -1;
    
}

int Philosopher::getRightFork() {
      if(state!=State::HUNGRY){
        printf("ph %d is not hungry in getRightFork",id);
        return -1;
    }
    bool success=false;
    int rightid=id+1;
    if(rightid==6){rightid=1;}
    Semaphores[rightid].P();
    if (lockstate[rightid]==-1){
        lockstate[rightid]=id;
        success=true;
    }
  
    if(success){
        rightFork=rightid;
        return rightFork;
    }

    return -1;
}

bool Philosopher::finish_eat() {
  if(state!=State::EATING){
        printf("ph %d is not eating in finish_eat",id);
        return false;
    }
    if(leftFork==-1||rightFork==-1){
        printf("ph %d does not have both forks in finish_eat",id);
        return false;
    }
    state = EATING;
    
    lockstate[leftFork]=-1;
    Semaphores[leftFork].V();
    leftFork=-1;
    
    lockstate[rightFork]=-1;
    Semaphores[rightFork].V();
    rightFork=-1;
    state = THINKING;
    return true;

}