#include "memory.h"
#include "os_constant.h"
#include "stdlib.h"
#include "asm_utils.h"
#include "stdio.h"
#include "program.h"
#include "os_modules.h"
#include "list.h"

static ListItem swapNodePool[SWAP_DATA_SLOTS];
static bool swapNodeUsed[SWAP_DATA_SLOTS];

ListItem* MemoryManager::allocateSwapListNode()
{
    for (int i = 0; i < SWAP_DATA_SLOTS; ++i)
    {
        if (!swapNodeUsed[i])
        {
            swapNodeUsed[i] = true;
            swapNodePool[i].prev = nullptr;
            swapNodePool[i].next = nullptr;
            swapNodePool[i].priority = 0;
            return &swapNodePool[i];
        }
    }
    return nullptr;
}

void MemoryManager::releaseSwapListNode(ListItem *node)
{
    if (!node)
        return;

    int index = node - swapNodePool;
    if (index < 0 || index >= SWAP_DATA_SLOTS)
        return;

    swapNodeUsed[index] = false;
    node->prev = nullptr;
    node->next = nullptr;
    node->priority = 0;
}

ListItem* MemoryManager::findSwapListNode(int virtualAddress)
{
    for (ListItem *item = swapList.front(); item != nullptr; item = item->next)
    {
        if (item->priority == virtualAddress)
            return item;
    }
    return nullptr;
}

MemoryManager::MemoryManager()
{
    initialize();
}

void MemoryManager::initialize()
{
    this->totalMemory = 0;
    this->totalMemory = getTotalMemory();
    swapList.initialize();

    // 预留的内存
    int usedMemory = 256 * PAGE_SIZE + 0x100000;
    int swappages = SWAP_DATA_SLOTS + 1; // 31 data pages + 1 metadata page
    usedMemory += swappages * PAGE_SIZE;
    if (this->totalMemory < usedMemory)
    {
        printf("memory is too small, halt.\n");
        asm_halt();
    }
    // 剩余的空闲的内存
    int freeMemory = this->totalMemory - usedMemory;

    int freePages = freeMemory / PAGE_SIZE;
    int kernelPages = freePages / 2;
    int userPages = freePages - kernelPages;

    //用于测试，先把kernelPages设置小一些，方便测试页表分配失败的情况
    kernelPages=16;

    int kernelPhysicalStartAddress = usedMemory;
    int userPhysicalStartAddress = usedMemory + kernelPages * PAGE_SIZE;

    int kernelPhysicalBitMapStart = BITMAP_START_ADDRESS;
    int userPhysicalBitMapStart = kernelPhysicalBitMapStart + ceil(kernelPages, 8);
    int kernelVirtualBitMapStart = userPhysicalBitMapStart + ceil(userPages, 8);
    int pageTablePhysicalBitMapStart = kernelVirtualBitMapStart + ceil(kernelPages, 8);
    int swapPhysicalBitMapStart=pageTablePhysicalBitMapStart+ceil(255, 8);
    int pageTablePages = 255;

    kernelPhysical.initialize(
        (char *)kernelPhysicalBitMapStart,
        kernelPages,
        kernelPhysicalStartAddress);

    userPhysical.initialize(
        (char *)userPhysicalBitMapStart,
        userPages,
        userPhysicalStartAddress);

    kernelVirtual.initialize(
        (char *)kernelVirtualBitMapStart,
        kernelPages,
        KERNEL_VIRTUAL_START);

    pageTablePhysical.initialize(
        (char *)pageTablePhysicalBitMapStart,
        pageTablePages,
        PAGE_DIRECTORY + PAGE_SIZE);

    swapPhysical.initialize((char *)swapPhysicalBitMapStart, swappages-1, 
        SWAP_START);
        
    printf("total memory: %d bytes ( %d MB )\n",
           this->totalMemory,
           this->totalMemory / 1024 / 1024);

    printf("kernel pool\n"
           "    start address: 0x%x\n"
           "    total pages: %d ( %d MB )\n"
           "    bitmap start address: 0x%x\n",
           kernelPhysicalStartAddress,
           kernelPages, kernelPages * PAGE_SIZE / 1024 / 1024,
           kernelPhysicalBitMapStart);

    printf("user pool\n"
           "    start address: 0x%x\n"
           "    total pages: %d ( %d MB )\n"
           "    bit map start address: 0x%x\n",
           userPhysicalStartAddress,
           userPages, userPages * PAGE_SIZE / 1024 / 1024,
           userPhysicalBitMapStart);

    printf("kernel virtual pool\n"
           "    start address: 0x%x\n"
           "    total pages: %d  ( %d MB ) \n"
           "    bit map start address: 0x%x\n",
           KERNEL_VIRTUAL_START,
           kernelPages, kernelPages * PAGE_SIZE / 1024 / 1024,
           kernelVirtualBitMapStart);

    printf("page table pool\n"
           "    start address: 0x%x\n"
           "    total pages: %d ( %d MB )\n"
           "    bit map start address: 0x%x\n",
           PAGE_DIRECTORY + PAGE_SIZE,
           pageTablePages, pageTablePages * PAGE_SIZE / 1024 / 1024,
           pageTablePhysicalBitMapStart);

    printf("swap pool\n"
           "    start address: 0x%x\n"
           "    total pages: %d ( %d MB )\n"
           "    bit map start address: 0x%x\n",
           SWAP_START,
           swappages-1, (swappages-1) * PAGE_SIZE / 1024 / 1024,
           swapPhysicalBitMapStart);
}

int MemoryManager::allocatePhysicalPages(enum AddressPoolType type, const int count)
{
    int start = -1;

    if (type == AddressPoolType::KERNEL)
    {
        start = kernelPhysical.allocate(count);
    }
    else if (type == AddressPoolType::USER)
    {
        start = userPhysical.allocate(count);
    }

    return (start == -1) ? 0 : start;
}

void MemoryManager::releasePhysicalPages(enum AddressPoolType type, const int paddr, const int count)
{
    if (type == AddressPoolType::KERNEL)
    {
        kernelPhysical.release(paddr, count);
    }
    else if (type == AddressPoolType::USER)
    {

        userPhysical.release(paddr, count);
    }
}

int MemoryManager::getTotalMemory()
{

    if (!this->totalMemory)
    {
        int memory = *((int *)MEMORY_SIZE_ADDRESS);
        // ax寄存器保存的内容
        int low = memory & 0xffff;
        // bx寄存器保存的内容
        int high = (memory >> 16) & 0xffff;

        this->totalMemory = low * 1024 + high * 64 * 1024;
    }

    return this->totalMemory;
}

void MemoryManager::openPageMechanism()
{
    // 页目录表指针
    int *directory = (int *)PAGE_DIRECTORY;
    //线性地址0~4MB对应的页表
    int *page = nullptr;
    // 预留页表区将第一个页表页作为低端页表使用
    int pageTablePage = pageTablePhysical.allocate(1);
    if (!pageTablePage)
    {
        printf("page table pool allocate failed\n");
        asm_halt();
    }
    page = (int *)pageTablePage;

    // 初始化页目录表
    memset(directory, 0, PAGE_SIZE);
    // 初始化线性地址0~4MB对应的页表
    memset(page, 0, PAGE_SIZE);

    int address = 0;
    // 将线性地址0~1MB恒等映射到物理地址0~1MB
    for (int i = 0; i < 256; ++i)
    {
        // U/S = 1, R/W = 1, P = 1
        page[i] = address | 0x7;
        address += PAGE_SIZE;
    }
    address= 256 * PAGE_SIZE + 0x100000;
    for(int i=0; i<32;++i){
        page[512+i]=address|0x7;
        address+=PAGE_SIZE;
    }


    // 初始化页目录项

    // 0~1MB
    directory[0] = ((int)page) | 0x07;
    // 3GB的内核空间
    directory[768] = directory[0];
    // 最后一个页目录项指向页目录表
    directory[1023] = ((int)directory) | 0x7;

    // 初始化cr3，cr0，开启分页机制
    asm_init_page_reg(directory);

    printf("open page mechanism\n");
}
 
int MemoryManager::allocatePages(enum AddressPoolType type, const int count)
{   
    memory_lock.lock();
    // 第一步：从虚拟地址池中分配若干虚拟页
    int virtualAddress = allocateVirtualPages(type, count);
    if (!virtualAddress)
    {
        memory_lock.unlock();   
        return 0;
    }

    bool flag;
    int physicalPageAddress;
    int vaddress = virtualAddress;

    // 依次为每一个虚拟页指定物理页
    for (int i = 0; i < count; ++i, vaddress += PAGE_SIZE)
    {
        flag = false;
        // 第二步：从物理地址池中分配一个物理页
        physicalPageAddress = allocatePhysicalPages(type, 1);
        if (physicalPageAddress)
        {
            //printf("allocate physical page 0x%x\n", physicalPageAddress);

            // 第三步：为虚拟页建立页目录项和页表项，使虚拟页内的地址经过分页机制变换到物理页内。
            flag = connectPhysicalVirtualPage(type, vaddress, physicalPageAddress);
        }
        else
        {
            flag = false;
        }

        // 分配失败，释放前面已经分配的虚拟页和物理页表
        if (!flag)
        {
            memory_lock.unlock();
            // 前i个页表已经指定了物理页
            releasePages(type, virtualAddress, i);
            // 剩余的页表未指定物理页
            releaseVirtualPages(type, virtualAddress + i * PAGE_SIZE, count - i);
            return 0;
        }
    }

    memory_lock.unlock();
    return virtualAddress;
}

int MemoryManager::allocateVirtualPages(enum AddressPoolType type, const int count)
{
    int start = -1;

    if (type == AddressPoolType::KERNEL)
    {
        start = kernelVirtual.allocate(count);
    }

    return (start == -1) ? 0 : start;
}

bool MemoryManager::connectPhysicalVirtualPage(enum AddressPoolType type, const int virtualAddress, const int physicalPageAddress)
{   
    // 计算虚拟地址对应的页目录项和页表项
    int *pde = (int *)toPDE(virtualAddress);
    int *pte = (int *)toPTE(virtualAddress);
    int pdeIndex = ((virtualAddress & 0xffc00000) >> 22);
    int pageTablePhys;

    // 页目录项无对应的页表，先分配一个页表
    if(!(*pde & 0x00000001)) 
    {
        // 从预留的页表区域中分配一个页表
        int page;
        if (pdeIndex == 0 || (pdeIndex >= 768 && pdeIndex <= 1022))
        {
            page = pageTablePhysical.allocate(1);
        }
        else
        {
            page = allocatePhysicalPages(AddressPoolType::KERNEL, 1);
        }
        if (!page)
            return false;

        // 使页目录项指向页表
        *pde = page | 0x7;
        pageTablePhys = page;
        // 初始化页表
        char *pagePtr = (char *)(((int)pte) & 0xfffff000);
        memset(pagePtr, 0, PAGE_SIZE);
        printf("alloc PDE %d -> page table phys 0x%x for vaddr 0x%x\n", pdeIndex, pageTablePhys, virtualAddress);
    }
    else
    {
        pageTablePhys = (*pde) & 0xfffff000;
        printf("use PDE %d -> page table phys 0x%x for vaddr 0x%x\n", pdeIndex, pageTablePhys, virtualAddress);
    }

    if (*pte & 0x00000001)
    {
        int oldPhysicalPage = (*pte) & 0xfffff000;
        if (oldPhysicalPage != physicalPageAddress)
        {
            releasePhysicalPages(type, oldPhysicalPage, 1);
        }
    }

    // 使页表项指向物理页
    *pte = physicalPageAddress | 0x7;

    return true;
}

unsigned int MemoryManager::toPDE(const int virtualAddress)
{
    return (0xfffff000 + (((virtualAddress & 0xffc00000) >> 22) * 4));
}

unsigned int MemoryManager::toPTE(const int virtualAddress)
{
    return (0xffc00000 + ((virtualAddress & 0xffc00000) >> 10) + (((virtualAddress & 0x003ff000) >> 12) * 4));
}

void MemoryManager::releasePages(enum AddressPoolType type, const int virtualAddress, const int count)
{   
    printf("releasePages start vaddr 0x%x count %d\n", virtualAddress, count);
    memory_lock.lock();
    unsigned int vaddr = (unsigned int)virtualAddress;
    unsigned int startPde = (vaddr & 0xffc00000) >> 22;
    unsigned int endPde = ((vaddr + (count - 1) * PAGE_SIZE) & 0xffc00000) >> 22;

    int *pte;
    for (int i = 0; i < count; ++i, vaddr += PAGE_SIZE)
    {
        // 第一步，对每一个虚拟页，释放为其分配的物理页
        pte = (int *)toPTE((int)vaddr);
        if (!(*pte & 0x00000001))
            continue;

        int paddr = (*pte) & 0xfffff000;
        releasePhysicalPages(type, paddr, 1);

        // 设置页表项为不存在，防止释放后被再次使用
        *pte = 0;
    }

    // 第二步，释放虚拟页
    releaseVirtualPages(type, virtualAddress, count);

    // 第三步，若页表已经空，则释放页表物理页并清除PDE
    for (unsigned int pdeIndex = startPde; pdeIndex <= endPde; ++pdeIndex)
    {
        unsigned int *pde = (unsigned int *)(0xfffff000 + pdeIndex * 4);
        if (!(*pde & 0x00000001))
            continue;

        int *pageTable = (int *)(0xffc00000 + (pdeIndex << 12));
        bool empty = true;
        for (int j = 0; j < 1024; ++j)
        {
            if (pageTable[j] & 0x00000001)
            {
                empty = false;
                break;
            }
        }

        if (empty)
        {
            int pageTablePhys = (*pde) & 0xfffff000;
            *pde = 0;
            if (pageTablePhys >= PAGE_DIRECTORY + PAGE_SIZE &&
                pageTablePhys < PAGE_DIRECTORY + PAGE_SIZE + 255 * PAGE_SIZE)
            {
                pageTablePhysical.release(pageTablePhys, 1);
            }
            else
            {
                releasePhysicalPages(AddressPoolType::KERNEL, pageTablePhys, 1);
            }
        }
    }
    memory_lock.unlock();
}

int MemoryManager::vaddr2paddr(int vaddr)
{
    int *pte = (int *)toPTE(vaddr);
    if(!(*pte & 0x00000001)) 
    {
        return -1;
    }
    int page = (*pte) & 0xfffff000;
    int offset = vaddr & 0xfff;
    return (page + offset);
}

void MemoryManager::releaseVirtualPages(enum AddressPoolType type, const int vaddr, const int count)
{
    if (type == AddressPoolType::KERNEL)
    {
        kernelVirtual.release(vaddr, count);
    }
}

int MemoryManager::swapOut(int virtualAddress)
{
    printf("swap out vaddr 0x%x\n", virtualAddress);
    if(virtualAddress < KERNEL_VIRTUAL_START) {
        printf("swap out failed: virtual address 0x%x is not in kernel space\n", virtualAddress);
        return 0;
    }
    if(virtualAddress %PAGE_SIZE != 0) {
        printf("swap out failed: virtual address 0x%x is not page aligned\n", virtualAddress);
        return 0;
    }
    int start = swapPhysical.allocate(1);
    if (start == 0)
    {
        printf("swap out failed: no swap slot\n");
        return 0;
    }

    int paddr = vaddr2paddr(virtualAddress);
    if (paddr == -1)
    {
        printf("swap out failed: virtual address 0x%x not mapped\n", virtualAddress);
        swapPhysical.release(start, 1);
        return 0;
    }

    int slotIndex = (start - SWAP_START) / PAGE_SIZE;
    int *meta = (int *)SWAP_META_START;
    meta[slotIndex] = virtualAddress;

    int *dist = (int *)start;
    int *src = (int *)virtualAddress;
    for (int i = 0; i < PAGE_SIZE / 4; ++i)
    {
        dist[i] = src[i];
    }

    // 清除原页映射，回收物理页帧
    int *pte = (int *)toPTE(virtualAddress);
    *pte = 0;
    releasePhysicalPages(AddressPoolType::KERNEL, paddr, 1);

    return start;
}

int MemoryManager::swapIn(int virtualAddress)
{
    printf("swap in vaddr 0x%x\n", virtualAddress);
    if(virtualAddress < KERNEL_VIRTUAL_START) {
        printf("swap in failed: virtual address 0x%x is not in kernel space\n", virtualAddress);
        return 0;
    }
    if(virtualAddress %PAGE_SIZE != 0) {
        printf("swap in failed: virtual address 0x%x is not page aligned\n", virtualAddress);
        return 0;
    }
     int *meta = (int *)SWAP_META_START;
    int index = -1;
    for (int i = 0; i < SWAP_DATA_SLOTS; ++i)
    {
        if (meta[i] == virtualAddress)
        {
            index = i;
            break;
        }
    }
    if (index == -1)
    {
        printf("swap in failed: no swap slot for vaddr 0x%x\n", virtualAddress);
        return 0;
    }
    // 为虚拟页分配物理页并建立映射
     int physicalPageAddress = allocatePhysicalPages(AddressPoolType::KERNEL, 1);
     if (!physicalPageAddress)
     {
         printf("swap in failed: no physical page\n");
         return 0;
     }
     if (!connectPhysicalVirtualPage(AddressPoolType::KERNEL, virtualAddress, physicalPageAddress))
     {
         releasePhysicalPages(AddressPoolType::KERNEL, physicalPageAddress, 1);
         printf("swap in failed: can not connect physical page\n");
         return 0;
     }



   
    int swapSlot = swapPhysical.startAddress + index * PAGE_SIZE;

    int *src = (int *)swapSlot;
    int *dist = (int *)virtualAddress;
    for (int i = 0; i < PAGE_SIZE / 4; ++i)
    {
        dist[i] = src[i];
    }

    // 清除交换区元数据和占用位
    meta[index] = 0;
    swapPhysical.release(swapSlot, 1);

     

     return virtualAddress;
}

int MemoryManager::allocatePages_swap(enum AddressPoolType type, const int count)
{
    memory_lock.lock();
    // 第一步：从虚拟地址池中分配若干虚拟页
    int virtualAddress = allocateVirtualPages(type, count);
    if (!virtualAddress)
    {
        memory_lock.unlock();
        return 0;
    }

    bool flag;
    int physicalPageAddress;
    int vaddress = virtualAddress;

    // 依次为每一个虚拟页指定物理页
    for (int i = 0; i < count; ++i, vaddress += PAGE_SIZE)
    {
        flag = false;
        physicalPageAddress = allocatePhysicalPages(type, 1);

        if (physicalPageAddress)
        {
            if (connectPhysicalVirtualPage(type, vaddress, physicalPageAddress))
            {
                ListItem *node = allocateSwapListNode();
                if (node)
                {
                    node->priority = vaddress;
                    swapList.push_back(node);
                    flag = true;
                }
                else
                {   
                    printf("allocate swap list node failed\n");
                    asm_halt();
                }
            }
            else
            {
               flag = false;
            }
        }

        if (!flag)
        {
            ListItem *victim = swapList.front();
            if (victim)
            {
                int victimVaddr = victim->priority;
                swapList.pop_front();
                int swapSlot = swapOut(victimVaddr);
                if (swapSlot != 0)
                {
                    physicalPageAddress = allocatePhysicalPages(type, 1);
                    if (physicalPageAddress)
                    {
                        if (connectPhysicalVirtualPage(type, vaddress, physicalPageAddress))
                        {
                            victim->priority = vaddress;
                            swapList.push_back(victim);
                            flag = true;
                        }
                        else
                        {
                            flag = false;
                            
                        }
                    }
                }

                if (!flag)
                {
                    swapList.push_front(victim);
                }
            }
        }

        if (!flag)
        {
            memory_lock.unlock();
            // 前i个页表已经指定了物理页
            releasePages(type, virtualAddress, i);
            // 剩余的页表未指定物理页
            releaseVirtualPages(type, virtualAddress + i * PAGE_SIZE, count - i);
            return 0;
        }
    }

    memory_lock.unlock();
    return virtualAddress;
}