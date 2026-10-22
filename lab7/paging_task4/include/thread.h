#ifndef THREAD_H
#define THREAD_H
#include "list.h"
#include "os_constant.h"
enum ProgramStatus
{
    CREATED,
    RUNNING,
    READY,
    BLOCKED,
    DEAD
};

enum BlockReason
{
    BLOCK_NONE,
    BLOCK_SLEEP,
    BLOCK_MUTEX,
    BLOCK_SEMAPHORE,
    BLOCK_WAIT_CHILD,
    BLOCK_IO
};

struct PCB{ 
    int *stack;                      // 栈指针，用于调度时保存esp
    char name[MAX_PROGRAM_NAME + 1]; // 线程名
    enum ProgramStatus status;       // 线程的状态
    int priority;                    // 线程优先级
    int pid;                          // 线程pid
    int ticks;                       // 线程时间片总时间
    int ticksPassedBy;               // 线程已执行时间
    int exitCode;                     // 线程退出码
    int parentPid;                    // 父线程pid
    enum BlockReason blockReason;       // 线程被阻塞的原因
    int stackTop;                      // 线程栈顶地址
    int stackBase;                     // 线程栈底地址
    ListItem tagInGeneralList;       // 线程队列标识
    ListItem tagInAllList;           // 线程队列标识
    int stackMagic;                    // 栈的魔数，用于检测栈溢出
   



};




#endif
