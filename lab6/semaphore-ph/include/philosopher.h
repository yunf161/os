#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

enum State { THINKING, HUNGRY, EATING };

class Philosopher {
    private:
        int id;
        State state;
        int leftFork;
        int rightFork;
    public:
        Philosopher();
        void initialize(int id);
        int getId();
        State getState();
        void setState(State newState);
        int getLeftFork();
        int getRightFork();
        int getLeftForkid();
        int getRightForkid();
        bool  finish_eat();

};

#endif