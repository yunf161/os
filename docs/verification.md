# 整理与验证说明

原始导出包含 665 个源码与教学说明文件，共 1241670 字节。只选择 C/C++、头文件、汇编、构建与调试脚本及 Markdown；原实验目录不作修改。

## 发布副本的最小修复

1. `lab6/spinlock-v2/include/asm_utils.h`：`asm_get_lock`、`asm_release_lock` 的参数声明保留 `volatile int*`，解决调用方传入 volatile 锁值时的 C++ 类型错误；汇编实现未改。
2. `lab8/src/3-changed/src/kernel/program.cpp`：恢复缺失的 `executeProcess` 函数前半段，使用该变体已有的 `findProgramByPid` 查找新进程，并在失败路径恢复中断状态。

这些修复只在整理副本中应用，便于复现；不代表对全部内核逻辑进行了安全或并发正确性审查。

## 结果与限制

README 列出的六个阶段编译通过；最终阶段使用 QEMU i386 无窗口启动，读取物理地址 `0xb8000` 对应的 VGA 文本确认进入课程欢迎界面。测试过程没有打开虚拟机窗口。

部分链接输出存在历史教学代码的 GNU-stack / RWX 警告，未将其解释为完整安全加固。未逐一编译全部变体，也未验证所有故障、死锁、进程回收和内存边界场景。带 bug/problem 名称的目录按教学原样保留。

`SOURCE-MANIFEST.json` 记录导出源文件的字节级 SHA-256；两处整理修复的文件另带原始哈希。Git 换行规范化可能改变检出后的字节；比较清单前应留意换行设置。
