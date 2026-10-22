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

static const int REPORT_OFFSET = 24325159 % PAGE_SIZE;
static const int REPORT_VALUE = 24325159;

void first_thread(void *arg)
{
    int virtualPage = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
    if (!virtualPage)
    {
        printf("first thread: failed to allocate kernel virtual page\n");
        asm_halt();
    }

    int virtualAddress = virtualPage + REPORT_OFFSET;
    int physicalAddress = memoryManager.vaddr2paddr(virtualAddress);

    printf("first thread: kernel virtual page start = 0x%x\n", virtualPage);
    printf("first thread: test virtual address = 0x%x, offset = 0x%x\n", virtualAddress, REPORT_OFFSET);
    printf("first thread: physical address = 0x%x\n", physicalAddress);

    if ((physicalAddress & 0xfff) != REPORT_OFFSET)
    {
        printf("first thread: page offset mismatch after address translation\n");
        asm_halt();
    }

    *((int *)virtualAddress) = REPORT_VALUE;
    printf("first thread: write %d to virtual address 0x%x\n", REPORT_VALUE, virtualAddress);
    printf("first thread: read back %d from virtual address 0x%x\n", *((int *)virtualAddress), virtualAddress);

    memoryManager.releasePages(AddressPoolType::KERNEL, virtualPage, 1);
    printf("first thread: release kernel virtual page starting at 0x%x\n", virtualPage);

    int *pte = (int *)memoryManager.toPTE(virtualAddress);
    if ((*pte & 0x1) == 0)
    {
        printf("first thread: page table entry for 0x%x has been cleared\n", virtualAddress);
    }
    else
    {
        printf("first thread: page table entry for 0x%x is still present\n", virtualAddress);
        asm_halt();
    }

    printf("LYF 24325159\n");
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

    // 内存管理器
    memoryManager.openPageMechanism();
    memoryManager.initialize();

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