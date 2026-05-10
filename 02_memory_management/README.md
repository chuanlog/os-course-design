# 内存管理

本实验包含两个部分：

- 动态分区分配：支持 `首次适应 FF` 和 `最佳适应 BF`
- 页面置换：支持 `FIFO` 和 `LRU`

## 功能说明

### 动态分区分配

支持交互式命令：

- `alloc 进程名 大小`：申请内存
- `free 进程名`：释放内存
- `show`：显示已分配分区和空闲分区
- `exit`：退出当前模块

### 页面置换

输入页框数量和页面访问序列后，程序会逐步输出：

- 当前访问页面
- 页框状态
- 是否发生缺页
- 缺页次数与缺页率

## 编译

```bash
cd 02_memory_management
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o memory_management
```

或使用 CMake：

```bash
cd 02_memory_management
cmake -S . -B build
cmake --build build
```

## 运行

```bash
./memory_management
```

## 运行菜单

- `1`：动态分区分配
- `2`：页面置换
- `0`：退出程序

## 页面置换示例输入

```text
2
3
12
1 2 3 4 1 2 5 1 2 3 4 5
3
```

含义：

- 先进入页面置换模块
- 页框数为 3
- 页面访问序列长度为 12
- 最后选择 `3` 表示同时对比 FIFO 和 LRU
