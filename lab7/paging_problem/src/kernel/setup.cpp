#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 内存管理锁
spinlock memory_lock;

void test_virtual_memory()
{
    printf("virtual memory test begin\n");

    int v1 = memoryManager.allocatePages(AddressPoolType::KERNEL, 770);
    // 770页是一个比较大的分配，可以看到769页目录项
    if (!v1)
    {
        printf("fail: allocatePages(1) returned 0\n");
        asm_halt();
    }
    printf("allocated 1 page at vaddr 0x%x\n", v1);

    int p1 = memoryManager.vaddr2paddr(v1);
    if (p1 == -1)
    {
        printf("fail: vaddr2paddr(0x%x) returned -1\n", v1);
        asm_halt();
    }
    printf("vaddr 0x%x -> paddr 0x%x\n", v1, p1);





    int v2 = memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
    if (!v2)
    {
        printf("fail: allocatePages(2) returned 0\n");
        asm_halt();
    }
    printf("allocated 2 pages at vaddr 0x%x\n", v2);

    int p2 = memoryManager.vaddr2paddr(v2);
    int p3 = memoryManager.vaddr2paddr(v2 + PAGE_SIZE);
    if (p2 == -1 || p3 == -1)
    {
        printf("fail: vaddr2paddr on allocated pages returned -1\n");
        asm_halt();
    }
    if (p2 == p3)
    {
        printf("fail: two different virtual pages map to same physical page\n");
        asm_halt();
    }
    printf("vaddr 0x%x -> paddr 0x%x, vaddr 0x%x -> paddr 0x%x\n", v2, p2, v2 + PAGE_SIZE, p3);



    memoryManager.releasePages(AddressPoolType::KERNEL, v2, 2);
    printf("released 2 pages at vaddr 0x%x\n", v2);

    if (memoryManager.vaddr2paddr(v2) != -1 || memoryManager.vaddr2paddr(v2 + PAGE_SIZE) != -1)
    {
        printf("fail: released pages still mapped\n");
        asm_halt();
    }
    printf("released pages are unmapped\n");

    int v3 = memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
    if (!v3)
    {
        printf("fail: allocatePages(2) after release returned 0\n");
        asm_halt();
    }
    printf("reallocated 2 pages at vaddr 0x%x\n", v3);

    memoryManager.releasePages(AddressPoolType::KERNEL, v1, 1);
    memoryManager.releasePages(AddressPoolType::KERNEL, v3, 2);
    printf("virtual memory test passed\n");
}

void first_thread(void *arg)
{
    test_virtual_memory();
    asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    interruptManager.enableTimeInterrupt();
    

    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();
    memory_lock.initialize();
    
    // 内存管理器
    memoryManager.initialize();
    memoryManager.openPageMechanism();
   
    
    
    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 3,-1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
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