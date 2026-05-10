# 文件系统模拟

本实验实现了一个简化文件系统模拟器，支持以下功能：

- 创建文件
- 写入文件
- 读取文件
- 删除文件
- 查看目录
- 查看空闲空间与位示图

程序采用“固定大小磁盘块 + 位示图”的方式管理存储空间，用于模拟文件组织和空闲空间回收机制。

## 支持命令

- `create 文件名`：创建空文件
- `write 文件名 内容`：写入文件内容，自动重新分配块
- `read 文件名`：读取文件内容
- `delete 文件名`：删除文件并回收块
- `ls`：查看当前目录
- `stat`：查看文件系统空间统计
- `bitmap`：查看块使用位示图
- `help`：查看帮助
- `exit`：退出

## 编译

```bash
cd 04_file_system
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o file_system
```

或使用 CMake：

```bash
cd 04_file_system
cmake -S . -B build
cmake --build build
```

## 运行

```bash
./file_system
```

## 启动后输入

程序启动后会先让你输入：

1. 磁盘块数量
2. 每个磁盘块大小（字节）

例如：

```text
16
8
```

表示：

- 一共 16 个磁盘块
- 每块大小为 8 字节

## 示例命令

```text
create a.txt
write a.txt hello_os
read a.txt
ls
bitmap
stat
delete a.txt
ls
exit
```
