# OS 课程设计代码库

> 姓名：陈永川
>
> 学号：202330450301
>
> 班级：计算机科学与技术 1 班

本仓库包含操作系统课程设计的四个基础模块和一个拓展模块。各目录均可独立编译、独立运行，详细说明见各目录下的 `README.md`。

## 目录概览

| 目录                           | 内容              | 运行方式       |
| ---------------------------- | --------------- | ---------- |
| `01_process_scheduling`      | 处理机调度模拟         | `make run` |
| `02_memory_management`       | 内存管理模拟          | `make run` |
| `03_process_synchronization` | 进程同步与并发控制模拟     | `make run` |
| `04_file_system`             | 文件系统模拟          | `make run` |
| `05_real_os`                 | 真实裸机操作系统 MiniOS | `make run` |

## 环境要求

### 基础四个模块

适用于：

- `01_process_scheduling`
- `02_memory_management`
- `03_process_synchronization`
- `04_file_system`

需要环境：

- `C++17` 编译器：`g++` / `clang++` / `MSVC`
- `cmake` 3.20 及以上
- `git`
- `make`，macOS / Linux 推荐使用
- `FLTK` 相关图形依赖，项目会通过 `CMake FetchContent` 自动拉取 FLTK 源码

Linux 如果缺少图形库依赖，可参考各模块 README 中的安装说明。

### 拓展模块 `05_real_os`

适用于：

- `05_real_os`

需要环境：

- `i686-elf-gcc`
- `i686-elf-binutils`
- `i686-elf-grub`
- `xorriso`
- `mtools`
- `qemu`

macOS 可使用 Homebrew 安装：

```bash
brew install i686-elf-binutils i686-elf-gcc i686-elf-grub
brew install xorriso mtools qemu
```

## 各目录功能与运行命令

### 1. `01_process_scheduling`

功能：

- 模拟处理机调度算法
- 展示进程执行顺序、等待时间、周转时间和响应时间
- 提供 GUI 和 CLI 两种运行方式

已实现：

- `FCFS`
- `SJF`
- `RR`
- `Priority`
- `MLQ`
- `MLFQ`

运行：

```bash
cd 01_process_scheduling
make run
```

运行 CLI：

```bash
make run-cli
```

### 2. `02_memory_management`

功能：

- 模拟动态分区分配
- 模拟页面置换算法
- 展示内存分配、回收、页框变化和缺页统计
- 提供 GUI 和 CLI 两种运行方式

已实现：

- 动态分区分配：`FF`、`BF`
- 页面置换：`FIFO`、`LRU`、`LFU`、`CLOCK`、`RANDOM`

运行：

```bash
cd 02_memory_management
make run
```

运行 CLI：

```bash
make run-cli
```

### 3. `03_process_synchronization`

功能：

- 模拟经典进程同步与互斥问题
- 展示线程事件、执行顺序和最终结果
- 提供 GUI 和 CLI 两种运行方式

已实现：

- 生产者-消费者
- 读者-写者
- 哲学家进餐

运行：

```bash
cd 03_process_synchronization
make run
```

运行 CLI：

```bash
make run-cli
```

### 4. `04_file_system`

功能：

- 模拟层次文件系统
- 支持文件和目录管理
- 支持文件内容编辑、空间统计和位示图查看
- 提供 GUI 和 CLI 两种运行方式

已实现：

- 格式化磁盘镜像
- 创建、删除、重命名文件和目录
- 编辑并保存文件内容
- 查看目录树、文件列表、位示图和空间使用情况

运行：

```bash
cd 04_file_system
make run
```

运行 CLI：

```bash
make run-cli
```

### 5. `05_real_os`

功能：

- 实现真实 x86 裸机操作系统 `MiniOS`
- 可生成 `minios.iso`
- 可在 `QEMU` 中启动运行
- 可挂载虚拟硬盘 `disk.img`
- 支持 Shell、文件系统和外部应用程序

已实现：

- VGA 文本终端
- 键盘输入
- 物理内存管理
- ATA 硬盘读写
- MiniFS 文件系统
- 类 Linux Shell 命令
- 外部应用自动编译和加载
- 汇编应用 `hello.bin`
- C 语言计算器应用 `calc.bin`

运行：

```bash
cd 05_real_os
make run
```

常用命令：

```bash
make          # 编译内核、应用、ISO 和磁盘镜像
make run      # 编译并启动 QEMU
make clean    # 清理构建产物
```

进入 `MiniOS` 后可尝试：

```text
help
ls
pwd
mkdir home
cd home
write note.txt hello_minios
cat note.txt
cd /
exec hello.bin
exec calc.bin
```

## Windows 运行说明

基础四个模块在 Windows 下推荐使用 `CMake` 和 `Visual Studio 2022`。

以 `01_process_scheduling` 为例：

```powershell
cd D:\your-path\os-course-design\01_process_scheduling
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release --target process_scheduling_gui
.\build\Release\process_scheduling_gui.exe
```

其他模块对应 GUI 目标名：

- `memory_management_gui`
- `process_synchronization_gui`
- `file_system_gui`

`05_real_os` 推荐在 macOS / Linux 环境下使用交叉编译工具链和 `QEMU` 运行；生成的 `minios.iso` 也可以挂载到 VMware 虚拟机中启动。

## 说明

- 每个目录下都有独立 README，可查看更详细的功能、结构和使用说明。
- 基础四个模块主要用于演示操作系统课程中的核心算法和机制。
- `05_real_os` 是拓展提升部分，用于演示可启动、可交互、可运行外部应用的真实裸机 OS。
- 01～04个基础模块在win上可以用cmake很方便的构建，在mac/linux上可以之间使用makefile构建，`05_real_os` 依赖 `i686-elf-gcc` 交叉编译工具链，Windows 环境编译配置较复杂，及其不推荐在win上编译，编译最好在mac或者linux系统上进行。
