#include "program.h"
#include "stdlib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "stdio.h"
#include "thread.h"
#include "os_modules.h"
#include "stdlib.h"

const int PCB_SIZE = 4096;                   // PCB的大小，4KB。
char PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_SET_STATUS[MAX_PROGRAM_AMOUNT];     // PCB的分配状态，true表示已经分配，false表示未分配。
const int priority_max=10; //优先级上限


PCB *ProgramManager::allocatePCB(const char*name, int pirority,int ticks,int parentPid,int exitCode)
{
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        if (!PCB_SET_STATUS[i])
        {
            PCB_SET_STATUS[i] = true;
            PCB *newPCB = (PCB *)((int)PCB_SET + PCB_SIZE * i);
            memset(newPCB, 0, sizeof(PCB)); // 初始化PCB内存
            newPCB->stack=(int *)((int)newPCB+PCB_SIZE);
            newPCB->stackTop=(int)newPCB+PCB_SIZE;
            newPCB->stackBase=(int)(&(newPCB->stackMagic)+1);
            newPCB->stackMagic=0x19870916;
            newPCB->status=CREATED;
            newPCB->parentPid=parentPid;
            newPCB->pid=i;
            newPCB->priority=pirority;
            newPCB->ticks=ticks;
            newPCB->blockReason=BLOCK_NONE;
            newPCB->exitCode=exitCode;

            for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i){
             newPCB->name[i] = name[i];
                        }
            newPCB->ticksPassedBy=0;
            return newPCB;
            
        }
    }

    return nullptr;
}

void ProgramManager::releasePCB(PCB *program)
{   
    printf("program %s with priority %d returned with exit code %d\n", program->name, program->priority, program->exitCode);
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;
}

ProgramManager::ProgramManager()
{
    this->initialize();
}
void ProgramManager::initialize()
{
    allPrograms.initialize();
    readyPrograms.initialize();
    running = nullptr;
    for(int i=0;i<MAX_PROGRAM_AMOUNT;i++){
        PCB_SET_STATUS[i]=false;}
    
}

int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority,int parentPid)
{   
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();
    PCB *thread = allocatePCB(name, priority, priority, parentPid,0);
    if (thread == nullptr){
        return -1;
    }
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms.push_back(&(thread->tagInGeneralList));

    // 恢复中断
    interruptManager.setInterruptStatus(status);

    return thread->pid;

}



void ProgramManager::schedule()
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        if (running == nullptr)
        {
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }

        if (running->status == ProgramStatus::DEAD)
        {
            releasePCB(running);
            running = nullptr;
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }

        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running != nullptr && running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        running->ticks = running->priority ;
        readyPrograms.push_back(&(running->tagInGeneralList));
    }
    else if (running != nullptr && running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    asm_switch_thread(cur, next); 

    interruptManager.setInterruptStatus(status);
}


void program_exit()
{
    PCB *thread = programManager.running;
    thread->status = ProgramStatus::DEAD;

    if (thread->pid)
    {
        programManager.schedule();
    }
    else
    {
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}


void ProgramManager::schedule_priority_rob() // 基于优先级的抢占式调度算法
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0) // 没有就绪线程,挂起管理器
    {
        if (running == nullptr)
        {
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }

        if (running->status == ProgramStatus::DEAD)
        {
            releasePCB(running);
            running = nullptr;
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }

        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running != nullptr && running->status == ProgramStatus::RUNNING)
    {   printf("rob execution\n"); //抢占调度
        running->status = ProgramStatus::READY; // 将当前线程状态设为就绪
        running->priority++;// 增加当前线程的优先级，防止饥饿
         if(running->priority>priority_max) running->priority=priority_max; //优先级上限
        running->tagInGeneralList.priority=running->priority;
         running->tagInAllList.priority=running->priority;
        readyPrograms.push_priority(&(running->tagInGeneralList));// 将当前线程重新加入就绪队列

    }
    else if (running != nullptr && running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }
//上下文切换
    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    asm_switch_thread(cur, next); 

    interruptManager.setInterruptStatus(status);
}


int ProgramManager::executeThread_priority(ThreadFunction function, void *parameter, const char *name, int priority,int parentPid,int ticks)
{   
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();
    PCB *thread = allocatePCB(name, priority, ticks, parentPid,0);
    if (thread == nullptr){
        return -1;
    }
    thread->stack -= 7;
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;
    thread->tagInAllList.priority=priority;
    thread->tagInGeneralList.priority=priority;
    allPrograms.push_priority(&(thread->tagInAllList)); //优先级调度
    readyPrograms.push_priority(&(thread->tagInGeneralList));
 //立即进行一次抢占式调度检测
    if (running != nullptr &&
    running->status == RUNNING &&
    thread->priority > running->priority)
{
    schedule_priority_rob();
}
    // 恢复中断
    interruptManager.setInterruptStatus(status);


    return thread->pid;

}


void ProgramManager::schedule_priority_normal() //正常时间片耗尽调用的调度函数
{
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyPrograms.size() == 0)
    {
        if (running == nullptr)
        {
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }

        if (running->status == ProgramStatus::DEAD)
        {
            releasePCB(running);
            running = nullptr;
            interruptManager.disableInterrupt();
            printf("halt\n");
            asm_halt();
        }
        else if(running->status == ProgramStatus::RUNNING)
        {   //为了测试需要，最后一个死循环的线程让它暂时用异常的exitcode退出
             running->status= DEAD;
             running->exitCode=1;
             releasePCB(running);
             running=nullptr;
             interruptManager.disableInterrupt();
             printf("halt\n");
             asm_halt();
            //running->ticks = running->priority * 10;
            //return ;
        }

        interruptManager.setInterruptStatus(status);
        return;
    }
     for (ListItem *item = readyPrograms.front(); item != nullptr; item = item->next)
        {
            PCB *waiting = ListItem2PCB(item, tagInGeneralList);
            if (waiting->priority < priority_max)
            {
                ++waiting->priority;
                waiting->tagInGeneralList.priority = waiting->priority;
                waiting->tagInAllList.priority = waiting->priority;
            }
        }//aging，增加所有就绪线程的优先级，防止饥饿
  
    if (running != nullptr && running->status == ProgramStatus::RUNNING)
    {
       
            //时间片耗尽，重新分配时间片
        running->status = ProgramStatus::READY;
        running->ticks =1 ;
        readyPrograms.push_priority(&(running->tagInGeneralList));
        
    }
    else if (running != nullptr && running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }

   //上下文切换

    ListItem *item = readyPrograms.front();
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    
    next->status = ProgramStatus::RUNNING;
    running = next;
    readyPrograms.pop_front();

    asm_switch_thread(cur, next); 

    interruptManager.setInterruptStatus(status);
}