# 进程同步与并发控制

本模块已经改造成 `多文件分层 + FLTK 图形界面 + CLI 备用入口` 的结构：

- 同步算法核心位于 `src/sync/`
- GUI 使用 `FLTK` 实现，轻量、开源、免费、跨平台
- GUI 通过 `CMake FetchContent` 自动拉取 FLTK
- 由于线程并发执行会和界面刷新产生竞争，程序采用“先完成模拟，再统一收集和展示结果”的方式

## 目录结构

```text
03_process_synchronization
├── CMakeLists.txt
├── Makefile
├── README.md
└── src
    ├── cli_main.cpp
    ├── main.cpp
    ├── gui
    │   ├── EventTable.cpp
    │   ├── EventTable.h
    │   ├── MainWindow.cpp
    │   └── MainWindow.h
    └── sync
        ├── DiningPhilosophersSimulator.cpp
        ├── DiningPhilosophersSimulator.h
        ├── EventRecorder.h
        ├── ProducerConsumerSimulator.cpp
        ├── ProducerConsumerSimulator.h
        ├── ReadersWritersSimulator.cpp
        ├── ReadersWritersSimulator.h
        ├── SimulationFacade.cpp
        ├── SimulationFacade.h
        ├── SyncTypes.cpp
        └── SyncTypes.h
```

## 已实现内容

- `生产者 - 消费者`
  - 使用 `mutex + condition_variable` 控制缓冲区满/空等待
- `读者 - 写者`
  - 使用 `mutex + condition_variable` 实现写者优先策略
- `哲学家进餐`
  - 使用 `mutex` 和固定拿叉顺序避免死锁

## GUI 功能

FLTK 图形界面支持：

- 在三个经典同步问题之间切换
- 分别配置线程数量、轮数、缓冲区容量、延迟等参数
- 点击“运行模拟”后先执行完整多线程过程，再统一展示结果
- 使用表格展示按时间记录的线程事件日志
- 在摘要区展示配置、统计结果和所使用的同步策略

## 编译前准备

### 必备工具

- `g++` 或 `clang++`
- `cmake` 3.20 及以上
- `git`

### Linux 额外依赖

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake git libx11-dev libxext-dev libxft-dev libxinerama-dev libxcursor-dev libxrender-dev libxfixes-dev libfontconfig1-dev libglu1-mesa-dev
```

#### Fedora

```bash
sudo dnf install gcc-c++ cmake git libX11-devel libXext-devel libXft-devel libXinerama-devel libXcursor-devel libXrender-devel libXfixes-devel fontconfig-devel mesa-libGLU-devel
```

## 一键编译运行

目录下已经提供了 [Makefile](03_process_synchronization/Makefile)，推荐直接使用：

```bash
cd 03_process_synchronization
make run
```

常用命令：

```bash
make help        # 查看所有目标
make gui         # 只编译 GUI
make run         # 编译并运行 GUI
make cli         # 只编译 CLI
make run-cli     # 编译并运行 CLI
make clean       # 删除 GUI 构建目录
make distclean   # 删除所有构建目录
```

## 编译运行 GUI

```bash
cd 03_process_synchronization
cmake -S . -B build -DBUILD_PROCESS_SYNCHRONIZATION_GUI=ON -DBUILD_PROCESS_SYNCHRONIZATION_CLI=ON
cmake --build build --target process_synchronization_gui -j
./build/process_synchronization_gui
```

## 编译运行 CLI

```bash
cd 03_process_synchronization
cmake -S . -B build-cli -DBUILD_PROCESS_SYNCHRONIZATION_GUI=OFF -DBUILD_PROCESS_SYNCHRONIZATION_CLI=ON
cmake --build build-cli --target process_synchronization_cli -j
./build-cli/process_synchronization_cli
```

## GUI 使用说明

1. 启动 `process_synchronization_gui`
2. 选择要模拟的同步问题
3. 调整线程数量、轮数、缓冲区容量和延迟等参数
4. 点击“运行模拟”
5. 程序会先完成多线程并发过程，再统一把事件日志和汇总结果更新到界面

## CLI 说明

CLI 会在运行前提示输入对应场景的参数，空输入时自动采用默认值。模拟完成后会输出：

- 线程事件日志
- 事件发生时间
- 场景摘要与同步策略说明
