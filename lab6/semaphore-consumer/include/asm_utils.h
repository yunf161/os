#ifndef ASM_UTILS_H
#define ASM_UTILS_H
#include "os_type.h"
#include "thread.h"

extern "C" void asm_hello_world();
extern "C" void asm_lidt(uint32 address, uint16 size);
extern "C" void asm_interrupt_empty_handler();
extern "C" void asm_halt();
extern "C" void asm_unhandled_interrupt();
extern "C" void asm_out_port(uint16 port, uint8 value);
extern "C" void asm_in_port(uint16 port, uint8 *value);
extern "C" void asm_time_interrupt_handler();
extern "C" void asm_enable_interrupt(); 
extern "C" void asm_disable_interrupt();
extern "C" int asm_interrupt_status();
extern "C" void asm_idle();
extern "C" void asm_switch_thread(PCB* current, PCB* next);
extern "C" void asm_atomic_exchange(int *lock_value, int *key);

#endif