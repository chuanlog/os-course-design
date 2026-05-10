# OS课程设计代码库

本仓库实现了操作系统课程设计基础必做的四个部分，全部使用 C++ 编写，并按模块拆分为 4 个独立文件夹。每个部分都提供了独立的 `README.md`，可以单独编译、单独运行。

## 目录结构

- `01_process_scheduling`：处理机调度
- `02_memory_management`：内存管理
- `03_process_synchronization`：进程同步与并发控制
- `04_file_system`：文件系统模拟

## 推荐环境

- macOS / Linux
- `g++` 10 及以上，支持 C++17
- `cmake` 3.10 及以上（可选）

## 快速运行

每个子目录都可以独立编译运行，示例：

```bash
cd 01_process_scheduling
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o scheduling
./scheduling
```

其它模块的运行方式请直接查看对应目录下的 `README.md`。
