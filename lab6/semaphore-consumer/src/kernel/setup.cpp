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
//信号量
Semaphore Asema;

Semaphore Bsema;

int MoCha_cake=0;
int Manguo_cake=0;

void server_A(void *arg){
    int times=2;
    while(times--){
        
    printf("Server A_LYF: start to make MoCha cake, there are %d MoCha cake now\n", 
           MoCha_cake);
           Asema.P();
    // make 10 MoCha_cake
    int temp=MoCha_cake;
    temp+=1;
    if(times==0){
        programManager.schedule();
    }
    MoCha_cake=temp;

    printf("Server A_LYF: Oh, Jesus! There are %d MoCha cake\n", MoCha_cake);
    Asema.V();
}
    
   

}
void server_B(void *arg){
    int times=2;
    while(times--){
        
    printf("Server_B_24325159 :start to make Manguo cake, there are %d Manguo cake now\n", 
           Manguo_cake);
    // make 10 Manguo_cake
    Bsema.P();
    int temp=Manguo_cake;
    temp+=1;
    if(times==0){
        programManager.schedule();
    }
    Manguo_cake=temp;

    printf("Server_B_24325159: Oh, Jesus! There are %d Manguo cake\n", Manguo_cake);
    Bsema.V();
}
    
}

void consumer_A(void *arg){
    int times=2;
    while(times--){
        printf("Consumer A_LYF: want to eat MoCha cake, there are %d MoCha cake now\n", 
           MoCha_cake);
        Asema.P();
        if(MoCha_cake>0){
            MoCha_cake--;
            printf("Consumer A_LYF: eat one MoCha cake, there are %d MoCha cake now\n", 
           MoCha_cake);
        }
        Asema.V();
    }
}

void consumer_B(void *arg){
    int times=2;
    while(times--){
        printf("Consumer_B_24325159: want to eat Manguo cake, there are %d Manguo cake now\n", 
           Manguo_cake);
        Bsema.P();
        if(Manguo_cake>0){
            Manguo_cake--;
            printf("Consumer_B_24325159: eat one Manguo cake, there are %d Manguo cake now\n", 
           Manguo_cake);
        }
        Bsema.V();
    }
}

void idle_thread(void *arg)
{
   printf("idle thread\n");
    Asema.initialize(1);
    Bsema.initialize(1);
   int pid=programManager.executeThread(server_B, nullptr, "server B", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    
    pid= programManager.executeThread(server_A, nullptr, "server A", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    pid= programManager.executeThread(consumer_A, nullptr, "consumer A", 1,0);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }
    pid= programManager.executeThread(consumer_B, nullptr, "consumer B", 1,0);
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
    printf("24325159 LYF");

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