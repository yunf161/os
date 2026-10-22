#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "os_modules.h"
#include "os_constant.h"
#include "memory.h"

static void assert_pool_state(AddressPool &pool, int begin, int end, bool expected, const char *message)
{
    for (int index = begin; index < end; ++index)
    {
        if (pool.get_status(index) != expected)
        {
            printf("%s: page %d status mismatch\n", message, index);
            asm_halt();
        }
    }
}

static int count_allocated_pages(AddressPool &pool)
{
    int allocated = 0;
    int totalPages = pool.resources.size();

    for (int index = 0; index < totalPages; ++index)
    {
        if (pool.get_status(index))
        {
            ++allocated;
        }
    }

    return allocated;
}

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 自旋锁
spinlock aLock;

Semaphore Semaphore_kernel;
Semaphore Semaphore_user;

MemoryManager memoryManager;

static void test_physical_pool(AddressPoolType type, AddressPool &pool, const char *name)
{
    int totalPages = pool.resources.size();
    int firstBatch = totalPages > 32 ? 10 : totalPages / 4;
    int secondBatch = totalPages > 32 ? 10 : totalPages / 4;
    int reuseBatch = totalPages > 16 ? 2 : 1;
    int tailBatch = totalPages > 32 ? 9 : totalPages / 5;

    if (firstBatch <= 0 || secondBatch <= 0 || reuseBatch <= 0 || tailBatch <= 0)
    {
        printf("%s thread: pool is too small for test\n", name);
        asm_halt();
    }

    printf("%s thread: physical page test begin, total pages %d\n", name, totalPages);

    int first = memoryManager.allocatePhysicalPages(type, firstBatch);
    int second = memoryManager.allocatePhysicalPages(type, secondBatch);
    if (!first || !second)
    {
        printf("%s thread: initial allocation failed\n", name);
        asm_halt();
    }

    if (first != pool.startAddress || second != pool.startAddress + firstBatch * PAGE_SIZE)
    {
        printf("%s thread: unexpected initial physical address\n", name);
        asm_halt();
    }

    assert_pool_state(pool, 0, firstBatch + secondBatch, true, "initial allocation");

    memoryManager.releasePhysicalPages(type, first, firstBatch);
    assert_pool_state(pool, 0, firstBatch, false, "release first batch");
    assert_pool_state(pool, firstBatch, firstBatch + secondBatch, true, "preserve second batch");

    int reused = memoryManager.allocatePhysicalPages(type, reuseBatch);
    if (reused != pool.startAddress)
    {
        printf("%s thread: released pages were not reused from pool head\n", name);
        asm_halt();
    }
    assert_pool_state(pool, 0, reuseBatch, true, "reuse released head");
    assert_pool_state(pool, reuseBatch, firstBatch, false, "head gap after partial reuse");

    int tail = memoryManager.allocatePhysicalPages(type, tailBatch);
    int expectedTail = pool.startAddress + (firstBatch + secondBatch) * PAGE_SIZE;
    if (tail != expectedTail)
    {
        printf("%s thread: contiguous allocation chose wrong gap\n", name);
        asm_halt();
    }
    assert_pool_state(pool, firstBatch + secondBatch, firstBatch + secondBatch + tailBatch, true, "tail allocation");

    memoryManager.releasePhysicalPages(type, second, secondBatch);
    memoryManager.releasePhysicalPages(type, reused, reuseBatch);
    memoryManager.releasePhysicalPages(type, tail, tailBatch);

    if (count_allocated_pages(pool) != 0)
    {
        printf("%s thread: leak after staged release\n", name);
        asm_halt();
    }

    int impossible = memoryManager.allocatePhysicalPages(type, totalPages + 1);
    if (impossible)
    {
        printf("%s thread: allocated more pages than pool owns\n", name);
        asm_halt();
    }

    int full = memoryManager.allocatePhysicalPages(type, totalPages);
    if (full != pool.startAddress)
    {
        printf("%s thread: full-pool allocation failed\n", name);
        asm_halt();
    }

    if (count_allocated_pages(pool) != totalPages)
    {
        printf("%s thread: full-pool allocation count mismatch\n", name);
        asm_halt();
    }

    if (memoryManager.allocatePhysicalPages(type, 1))
    {
        printf("%s thread: allocation succeeded after pool exhaustion\n", name);
        asm_halt();
    }

    memoryManager.releasePhysicalPages(type, full, totalPages);
    if (count_allocated_pages(pool) != 0)
    {
        printf("%s thread: leak after full release\n", name);
        asm_halt();
    }

    printf("%s thread: physical page test passed\n", name);
}

void kernel_thread_single(void *arg)
{
    test_physical_pool(KERNEL, memoryManager.kernelPhysical, "kernel");
}

void user_thread_single(void *arg)
{
    test_physical_pool(USER, memoryManager.userPhysical, "user");
}

void kernel_thread_multiple(void *arg){
    printf("kernel thread %d asks for 10 pages\n", *((int *)arg));
     int re=   memoryManager.allocatePhysicalPages(KERNEL, 10);
    if(!re){
        printf("kernel thread %d: can not allocate kernel physical pages\n %d\n", re);
        asm_halt();}
    int count = count_allocated_pages(memoryManager.kernelPhysical);
    printf("kernel thread %d: %d pages allocated\n", *((int *)arg), count);
}
void idle_thread(void *arg)
{
   printf("idle thread\n");
   int memory = *((uint32 *)MEMORY_SIZE_ADDRESS);
    // ax寄存器保存的内容
    int low = memory & 0xffff;
    // bx寄存器保存的内容
    int high = (memory >> 16) & 0xffff;
    memory = low * 1024 + high * 64 * 1024;
    printf("total memory: %d bytes (%d MB)\n", memory, memory / 1024 / 1024);
    int pid = programManager.executeThread(kernel_thread_single, nullptr, "kernel thread single", 3, -1);
    if(pid == -1)
    {
        printf("can not initialize kernel thread\n");
        asm_halt();
    }
    pid = programManager.executeThread(user_thread_single, nullptr, "user thread single", 3, -1);
    if(pid == -1)  {
        printf("can not initialize user thread\n");}

   


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
    Semaphore_kernel.initialize(1);
    Semaphore_user.initialize(1);
    printf("24325159 LYF");
    printf("physical memory test\n");

    memoryManager.initialize();

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