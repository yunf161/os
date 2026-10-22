#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

void forth_thread(void *arg)
{
    printf("pid %d name \"%s\": i am the forth therad!\n", 
           programManager.running->pid, programManager.running->name);
}

void first_thread(void *arg)
{
    printf("pid %d name \"%s\": Hello World!\n", 
           programManager.running->pid, programManager.running->name);
}

void second_thread(void *arg)
{    //测试优先级调度和抢占式调度，这个线程会在运行20次后创建一个优先级更高的线程，观察它是否会抢占second_thread的执行权
    int st=interruptManager.getInterruptStatus();
    bool status = interruptManager.getInterruptStatus();
    printf("pid %d name \"%s\": i am the second therad with initial priority %d!\n", 
           programManager.running->pid, programManager.running->name, programManager.running->priority);
    
    int count=0;

    while(count<500){
        if(count==20)
        {   //创建一个优先级更高的线程
        bool status = interruptManager.getInterruptStatus();
          int pid = programManager.executeThread_priority(forth_thread, nullptr, "forth thread", 6,2,100);
              if (pid == -1)
             {
            printf("can not execute thread\n");
              asm_halt();
         }
         ListItem *item = programManager.readyPrograms.front();
         PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
        //printf("pid %d name \"%s\ \n",  firstThread->pid, firstThread->name);
         

            
        }
        ++count;
    }
    //恢复执行的标志
    printf("pid %d name \"%s\": i am the second therad with priority %d after execution!\n", 
           programManager.running->pid, programManager.running->name, programManager.running->priority);
    while(true);
}

void idle_thread(void *arg)
{
    while (true)
    {
        asm_idle();
    }
}

void third_thread(void *arg)
{
    printf("pid %d name \"%s\": i am the third therad!\n", 
           programManager.running->pid, programManager.running->name);
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

    int pid = programManager.executeThread_priority(first_thread, nullptr, "first thread", 3,-1,1);
    if (pid == -1)
    {
        printf("can not initialize main thread\n");
        asm_halt();
    }
     // 创建第二个线程 
    pid = programManager.executeThread_priority(second_thread, nullptr, "second thread", 2,-1,2);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    pid = programManager.executeThread_priority(third_thread, nullptr, "third thread", 1,-1,1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    //按照混乱的优先级创建三个线程，看看是否会按照优先级排序
    
    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);
    
    
    asm_halt();
}