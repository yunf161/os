#ifndef OS_MODULES_H
#define OS_MODULES_H

#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "sync.h"

extern InterruptManager interruptManager;
extern STDIO stdio;
extern ProgramManager programManager;
extern spinlock aLock;

#endif