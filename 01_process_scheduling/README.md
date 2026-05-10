# 处理机调度

本模块已经改造成 `多文件分层 + FLTK 图形界面 + CLI 备用入口` 的结构：

- 调度核心算法位于 `src/scheduling/`
- GUI 使用 `FLTK` 实现，轻量、开源、免费、跨平台
- GUI 通过 `CMake FetchContent` 自动拉取 FLTK，尽量减少额外安装步骤
- CLI 保留为备用入口，便于在没有 GUI 依赖时验证调度逻辑

## 目录结构

```text
01_process_scheduling
├── CMakeLists.txt
├── Makefile
├── README.md
└── src
    ├── cli_main.cpp
    ├── main.cpp
    ├── gui
    │   ├── GanttChartWidget.cpp
    │   ├── GanttChartWidget.h
    │   ├── MainWindow.cpp
    │   ├── MainWindow.h
    │   ├── ResultsTable.cpp
    │   └── ResultsTable.h
    └── scheduling
        ├── FCFSScheduler.cpp
        ├── FCFSScheduler.h
        ├── MLFQScheduler.cpp
        ├── MLFQScheduler.h
        ├── MLQScheduler.cpp
        ├── MLQScheduler.h
        ├── PriorityScheduler.cpp
        ├── PriorityScheduler.h
        ├── RRScheduler.cpp
        ├── RRScheduler.h
        ├── SJFScheduler.cpp
        ├── SJFScheduler.h
        ├── SchedulerFacade.cpp
        ├── SchedulerFacade.h
        ├── SchedulingTypes.h
        ├── SchedulingUtils.cpp
        └── SchedulingUtils.h
```

## 已实现算法

- `FCFS`：先来先服务
- `SJF`：短作业优先，非抢占
- `RR`：时间片轮转
- `Priority`：优先级调度，非抢占，数值越小优先级越高
- `MLQ`：多级队列调度
- `MLFQ`：多级反馈队列调度

## MLQ 规则说明

当前实现的多级队列调度采用固定 3 个队列类型，但支持动态配置每个队列的调度优先级和时间片：

- 高队列：处理 `priority <= 2` 的进程
- 中队列：处理 `priority <= 4` 的进程
- 低队列：处理 `priority >= 5` 的进程
- `调度优先级`：数值越小，队列越先被调度
- `时间片`：大于 `0` 时按时间片轮转，等于 `0` 时按 `FCFS` 方式运行

## MLFQ 规则说明

当前实现的多级反馈队列调度采用固定 3 个反馈队列，并支持为每个队列独立设置：

- `调度算法`：可选 `FCFS`、`SJF`、`RR`、`Priority`
- `时间片`：必须大于 `0`

MLFQ 的行为规则如下：

- 新到达进程先进入高队列
- 若当前队列时间片用完且进程仍未完成，则下降到下一队列
- 高级队列可抢占低级队列
- 最低队列不会继续降级，只会在本队列按所选算法继续调度

## GUI 功能

FLTK 图形界面支持：

- 录入进程名称、到达时间、运行时间、优先级
- 添加进程、删除选中进程、清空列表、加载示例数据
- 输入进程个数后一键随机生成进程数据
- 选择单个算法运行
- 根据所选算法动态显示配置项
- 选择 `RR` 时显示单独时间片配置
- 选择 `MLQ` 时显示三个队列的调度优先级和时间片配置
- 选择 `MLFQ` 时显示三个队列的调度算法和时间片配置
- 通过列表选择进程，并在右侧查看选中进程信息
- 展示 Gantt 图
- 使用 FLTK 表格控件展示详细调度结果
- 使用可滚动摘要区展示长文本结果

## 编译前准备

### 必备工具

- `g++` 或 `clang++`
- `cmake` 3.20 及以上
- `git`

### Linux 额外依赖

虽然 FLTK 会通过 `FetchContent` 自动下载源码，但 Linux 下通常还需要基本图形系统开发包。

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

目录下已经提供了 [Makefile](file:///Users/bytedance/GitRepos/os-course-design/01_process_scheduling/Makefile)，推荐直接使用：

```bash
cd /Users/bytedance/GitRepos/os-course-design/01_process_scheduling
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
cd /Users/bytedance/GitRepos/os-course-design/01_process_scheduling
cmake -S . -B build -DBUILD_PROCESS_SCHEDULING_GUI=ON -DBUILD_PROCESS_SCHEDULING_CLI=ON
cmake --build build --target process_scheduling_gui -j
./build/process_scheduling_gui
```

## 编译运行 CLI

```bash
cd /Users/bytedance/GitRepos/os-course-design/01_process_scheduling
cmake -S . -B build-cli -DBUILD_PROCESS_SCHEDULING_GUI=OFF -DBUILD_PROCESS_SCHEDULING_CLI=ON
cmake --build build-cli --target process_scheduling_cli -j
./build-cli/process_scheduling_cli
```

## GUI 使用说明

1. 启动 `process_scheduling_gui`
2. 点击“载入示例数据”，或输入随机个数后点击“随机生成”，也可以手动输入进程信息后点击“添加进程”
3. 在左侧列表中选择要查看的进程，右侧会显示其信息
4. 选择调度算法
5. 如果选择 `RR`，设置时间片
6. 如果选择 `MLQ`，设置高/中/低队列的调度优先级和时间片
7. 如果选择 `MLFQ`，设置高/中/低队列的调度算法和时间片
8. 点击“运行模拟”
9. 在界面中查看 Gantt 图、详细结果和结果摘要

## CLI 示例输入

### MLFQ 示例

```text
5
P1 0 5 1
P2 1 4 3
P3 2 7 5
P4 3 2 2
P5 4 6 6
6
3 2
2 4
1 8
```

上面示例表示：

- 选择 `MLFQ`
- 高队列使用 `RR`，时间片 `2`
- 中队列使用 `SJF`，时间片 `4`
- 低队列使用 `FCFS`，时间片 `8`
