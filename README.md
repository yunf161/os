# x86 教学操作系统实验

基于操作系统课程框架完成的分阶段实验，使用 C/C++ 和 NASM 汇编，在 QEMU i386 环境中学习引导、中断、线程调度、同步、分页、用户进程和系统调用。开发环境为 Ubuntu / WSL。

本仓库是课程学习与修改记录，不是从零编写的 Linux 内核，也不是可用于生产的操作系统发行版。Linux 与 BusyBox 的早期构建实验仅保留自写的 `helloworld.c`；上游 Linux、BusyBox 完整源码和生成文件不入库。

## 模块与源码入口

| 模块 | 内容 | 入口 |
| --- | --- | --- |
| 汇编与启动 | MBR、磁盘读取、实模式与保护模式切换 | `lab2src`、`lab3src` |
| 混合编程与中断 | 汇编/C 调用及中断相关实验 | `lab4src` |
| 线程调度 | PCB、就绪队列、时钟中断、上下文切换 | `lab5/hw1/task_2_3_4` |
| 同步互斥 | 自旋锁、信号量、生产者消费者、哲学家实验 | `lab6` |
| 内存管理 | 位图地址池、物理页与虚拟页管理、分页 | `lab7` |
| 进程与系统调用 | TSS、用户态切换、int 0x80、fork/exit/wait | `lab8/src` |

各子目录是独立阶段与变体，不是自动集成为一个内核的所有功能。带 `problem`、`bug` 的目录包含问题分析样例，不应当作为可交付正确实现。`lab8/src/7` 的 Shell 目前为静态欢迎界面，不是完整交互命令解释器。

## 构建与运行

依赖：`make`、`nasm`、支持 32 位 freestanding 编译的 `gcc/g++`、GNU `ld/objcopy`、`qemu-system-i386`；调试使用 `gdb`。本次在现有 Ubuntu 24.04 环境验证，不在脚本中自动安装系统软件。

```bash
# 项目根目录，编译最终展示阶段并在缺失时创建空白镜像
bash scripts/build-stage.sh lab8/src/7

# 启动虚拟机
qemu-system-i386 -drive file=lab8/src/7/run/hd.img,format=raw -no-reboot

# 也可以选择其他已验证阶段
bash scripts/build-stage.sh lab7/paging
```

已有 Makefile 使用相对路径，并含多个目标共用编译命令；请按顺序构建，不使用 `make -j`。磁盘镜像和编译结果默认忽略，不随仓库下载。

GDB 调试示例（两个终端）：

```bash
qemu-system-i386 -drive file=lab8/src/7/run/hd.img,format=raw -S -s -no-reboot
cd lab8/src/7/run
gdb -x gdbinit
```

部分早期 `make debug` 目标依赖 `gnome-terminal`；在无桌面 WSL 中请使用上述两个终端的方式。

## 验证范围

此次整理在隔离副本中顺序编译以下六个阶段，均成功：

- `lab5/hw1/task_2_3_4`
- `lab6/spinlock-v2`
- `lab7/paging`
- `lab8/src/3-changed`
- `lab8/src/6-changed`
- `lab8/src/7`

最终阶段通过无窗口 QEMU 启动，使用 QMP 读取 VGA 文本内存，确认显示 `Welcome to SUMMER Project!`。该结果证明该展示链路能够启动，不等于所有调度、同步、内存及进程异常路径都已测试通过。

发布副本修复了两处编译问题，见 [整理与验证说明](docs/verification.md)。原始 WSL 实验目录保持不变。源码包括原工作区尚未提交的 lab7 修改及 lab8 内容；不复制原 Git 历史。

## 来源与贡献边界

课程框架及教学资料源于 SUMMER / SYSU 操作系统课程项目，相关地址与作者信息保留在原教学源码中：

https://gitee.com/nelsoncheung/sysu-2021-spring-operating-system/

本仓库保留框架代码、实验复现和学习修改，不将上游代码声明为个人原创。第三方来源说明见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。课程 README 中部分插图不随此次源码整理附带，阅读完整课程资料请访问原课程来源。
