#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "os_modules.h"
#include "philosopher.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 信号量
Semaphore Semaphores[6];
int lockstate[6];//-1表示叉子空闲，0-4表示被哲学家占用
Philosopher philosophers[6];
void phs(void *arg){
    int id=*(int*)arg;
   philosophers[id].setState(HUNGRY);
   printf("philosopher %d is hungry LYF 24325159\n", philosophers[id].getId());
   if(id%2!=0){
    philosophers[id].getLeftFork();
    printf("philosopher %d got left fork %d\n", philosophers[id].getId(), philosophers[id].getLeftForkid());
    programManager.schedule();
    philosophers[id].getRightFork();
    printf("philosopher %d got right fork %d\n", philosophers[id].getId(), philosophers[id].getRightForkid());
   } else{
    philosophers[id].getRightFork();
    printf("philosopher %d got right fork %d\n", philosophers[id].getId(), philosophers[id].getRightForkid());
    programManager.schedule();
    philosophers[id].getLeftFork();
    printf("philosopher %d got left fork %d\n", philosophers[id].getId(), philosophers[id].getLeftForkid());
    }
    philosophers[id].setState(EATING);
    bool success = philosophers[id].finish_eat();
    if (success) {
        printf("philosopher %d finished eating\n", philosophers[id].getId());
    }
    else {
        printf("philosopher %d failed to finish eating\n", philosophers[id].getId());
    }
}


void idle_thread(void *arg)
{  

    int arr[6];
  
     for(int i=1;i<6;i++){
       arr[i] = i;
       char name[2];
         name[0]='0'+i;
         name[1]='\0';
          int pid = programManager.executeThread(phs, &arr[i], name, 1,0);
          if (pid == -1)
          {
                printf("can not execute thread\n");
                asm_halt();
          }
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
    for(int i=1;i<6;i++){
        Semaphores[i].initialize(1);
        lockstate[i]=-1;
        philosophers[i].initialize(i);
    }
    printf("24325159 LYF\n");

    int pid = programManager.executeThread(idle_thread, nullptr, "idle thread", 3,-1);
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