#ifndef INTERRUPT_H
#define INTERRUPT_H
#include "os_type.h"
class InterruptManager
{
private:
    // IDT起始地址
    uint32 *IDT;
    uint32 IRQ0_8259A_MASTER; // 主片中断起始向量号
    uint32 IRQ0_8259A_SLAVE;  // 从片中断起始向量号
    
public:
    InterruptManager();
    // 初始化
    void initialize();

    // 设置中断描述符
// index   第index个描述符，index=0, 1, ..., 255
// address 中断处理程序的起始地址
// DPL     中断描述符的特权级
    void setInterruptDescriptor(uint32 index, uint32 address, byte DPL);//输入中断脚标（中断在数组中的位置索引，也就是中断号），中断处理程序地址，dpl（特权级）

      // 开启时钟中断
    void enableTimeInterrupt();
    // 禁止时钟中断
    void disableTimeInterrupt();
    // 设置时钟中断处理函数
    void setTimeInterrupt(void *handler);

private:
    // 初始化8259A芯片
    void initialize8259A();
};

#endif