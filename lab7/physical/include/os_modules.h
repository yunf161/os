#ifndef OS_MODULES_H
#define OS_MODULES_H

#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "sync.h"
#include "memory.h"

extern InterruptManager interruptManager;
extern STDIO stdio;
extern ProgramManager programManager;
extern spinlock aLock;
extern Semaphore Semaphore_kernel;
extern Semaphore Semaphore_user;
extern MemoryManager memoryManager;

#endif