# OS课程设计代码库

> 姓名：陈永川
>
> 学号：202330450301
>
> 班级：计算机科学与技术1班

本仓库实现了操作系统课程设计基础必做的四个部分，全部使用 C++ 编写，并按模块拆分为 4 个独立文件夹。每个部分都提供了独立的 `README.md`，可以单独编译、单独运行。

## 目录结构

- `01_process_scheduling`：处理机调度
- `02_memory_management`：内存管理
- `03_process_synchronization`：进程同步与并发控制
- `04_file_system`：文件系统模拟

## 项目概览

本仓库统一采用以下技术方案：

- 编程语言：`C++17`
- 构建系统：`CMake`
- 快速启动：各模块提供 `Makefile`
- 图形界面：`FLTK 1.4.x`
- GUI 依赖管理：`CMake FetchContent`

四个模块都采用了“核心算法/逻辑 + GUI 展示 + CLI 备用入口”的结构，便于课程设计演示、截图和答辩。

## 四个部分简介

### 1. 处理机调度

对应目录：[01\_process\_scheduling](file:///Users/bytedance/GitRepos/os-course-design/01_process_scheduling)

- 功能：模拟不同进程调度算法的执行顺序、周转时间、等待时间、响应时间，并通过 Gantt 图和结果表格展示调度过程。
- 已实现算法：
  - `FCFS`
  - `SJF`
  - `RR`
  - `Priority`
  - `MLQ`
  - `MLFQ`
- GUI 特点：
  - 支持进程录入、随机生成、动态配置运行参数
  - 使用 FLTK 自绘 Gantt 图和表格控件展示结果

### 2. 内存管理

对应目录：[02\_memory\_management](file:///Users/bytedance/GitRepos/os-course-design/02_memory_management)

- 功能：模拟动态分区分配与页面置换，展示分配/回收过程、缺页统计和页框变化。
- 已实现算法：
  - 动态分区分配：`FF`、`BF`
  - 页面置换：`FIFO`、`LRU`、`LFU`、`CLOCK`、`RANDOM`
- GUI 特点：
  - 支持随机生成申请序列、页框数和访问序列
  - 页面访问序列生成考虑了程序访问的局部性原理
  - 使用图形化内存布局和表格展示模拟过程

### 3. 进程同步与并发控制

对应目录：[03\_process\_synchronization](file:///Users/bytedance/GitRepos/os-course-design/03_process_synchronization)

- 功能：模拟经典同步互斥问题，记录并发执行过程中的线程事件和最终结果。
- 已实现问题：
  - `生产者-消费者`
  - `读者-写者`
  - `哲学家进餐`
- 已使用同步机制：
  - `std::thread`
  - `std::mutex`
  - `std::condition_variable`
- GUI 特点：
  - 先执行完整多线程模拟，再统一收集并展示结果
  - 使用事件表格展示线程动作顺序和时间信息

### 4. 文件系统模拟

对应目录：[04\_file\_system](file:///Users/bytedance/GitRepos/os-course-design/04_file_system)

- 功能：模拟简化层次文件系统，支持文件/目录管理、内容编辑、空间统计和位示图查看。
- 已实现内容：
  - 格式化磁盘镜像
  - 创建/删除/重命名文件与文件夹
  - 编辑并保存文本文件
  - 目录树浏览、文件列表查看、位示图与空间统计
- 核心机制：
  - 单个二进制镜像文件
  - 位示图管理空闲块
  - 固定目录项表
  - 数据块分配与回收
- GUI 特点：
  - 界面风格参考 Windows 文件管理器
  - 左侧目录树、右侧文件列表、下方内容编辑区

## 推荐环境

- `macOS / Linux / Windows`
- `g++` / `clang++` / `MSVC`，支持 `C++17`
- `cmake` 3.20 及以上
- `git`

Linux 下如果要运行 GUI，还需要基础图形依赖；各模块的 `README.md` 中已经给出 Ubuntu / Fedora 的安装命令。

## 快速运行

### macOS / Linux

推荐直接进入对应模块目录，用 `make run` 一键编译并启动 GUI：

```bash
cd /Users/bytedance/GitRepos/os-course-design/01_process_scheduling
make run
```

其他模块只需要替换目录名：

```bash
cd /Users/bytedance/GitRepos/os-course-design/02_memory_management
make run

cd /Users/bytedance/GitRepos/os-course-design/03_process_synchronization
make run

cd /Users/bytedance/GitRepos/os-course-design/04_file_system
make run
```

如果只想运行命令行版本：

```bash
make run-cli
```

### Windows

Windows 默认通常没有 GNU `make`，推荐直接使用 `CMake`。

前提：

- 安装 `Visual Studio 2022` 或 `Build Tools for Visual Studio 2022`
- 勾选 `Desktop development with C++`
- 安装 `CMake`
- 安装 `Git`

以 `01_process_scheduling` 为例：

```powershell
cd D:\your-path\os-course-design\01_process_scheduling
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release --target process_scheduling_gui
.\build\Release\process_scheduling_gui.exe
```

其他模块只需要把目标名替换成对应可执行文件：

- `memory_management_gui`
- `process_synchronization_gui`
- `file_system_gui`

如果只想编译 CLI，可关闭 GUI 选项：

```powershell
cmake -S . -B build-cli -G "Visual Studio 17 2022" -DBUILD_FILE_SYSTEM_GUI=OFF -DBUILD_FILE_SYSTEM_CLI=ON
cmake --build build-cli --config Release --target file_system_cli
.\build-cli\Release\file_system_cli.execmake -S . -B build-cli -G "Visual Studio 17 2022" -DBUILD_FILE_SYSTEM_GUI=OFF -DBUILD_FILE_SYSTEM_CLI=ON
cmake --build build-cli --config Release --target file_system_cli
.\build-cli\Release\file_system_cli.ex
```

### 通用 CMake 方式

如果你不使用 `make`，也可以在 macOS / Linux 上直接使用 `CMake`：

```bash
cd /Users/bytedance/GitRepos/os-course-design/04_file_system
cmake -S . -B build
cmake --build build --target file_system_gui -j
./build/file_system_gui
```

不同模块只需要替换：

- 目录名
- 构建目标名

## 说明

- 每个模块都可以独立编译和运行。
- 每个模块目录下都有更详细的使用文档和运行示例。
- GUI 统一基于 `FLTK`，便于跨平台编译。

