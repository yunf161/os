#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "os_modules.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 自旋锁
spinlock aLock;

Semaphore aSemaphore;

int cheese_burger=0;

void second_thread(void *arg){
     aSemaphore.P();
    
      int delay = 0;

    printf("mother: start to make cheese burger, there are %d cheese burger now\n", 
           cheese_burger);
    // make 10 cheese_burger
    cheese_burger += 10;

    printf("mother: oh, I have to hang clothes out.\n");
    // hanging clothes out
    delay = 0xfffffff;
    while (delay)
        --delay;
    // done

    printf("mother: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);
    
    aSemaphore.V();

}
void third_thread(void *arg){
    aSemaphore.P();
    printf("boy   : Look what I found!\n");
    // eat all cheese_burgers out secretly
    cheese_burger -= 10;    


    // run away as fast as possible
    aSemaphore.V();
}

void first_thread(void *arg)
{
   printf("idle thread\n");
    aLock.initialize();
   int pid=programManager.executeThread(second_thread, nullptr, "second thread", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    
    pid= programManager.executeThread(third_thread, nullptr, "third thread", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

   asm_halt();

}



extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();
    aSemaphore.initialize(1);
    printf("24325159 LYF");
    printf("semaphore test\n");

    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 3,-1);
    if (pid == -1)
    {
        printf("can not initialize main thread\n");
        asm_halt();
    }
     
    
    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);
    
    
    asm_halt();
}