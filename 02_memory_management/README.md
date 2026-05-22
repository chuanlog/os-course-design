# 内存管理

本模块已经改造成 `多文件分层 + FLTK 图形界面 + CLI 备用入口` 的结构：

- 内存管理核心算法位于 `src/memory/`
- GUI 使用 `FLTK` 实现，轻量、开源、免费、跨平台
- GUI 通过 `CMake FetchContent` 自动拉取 FLTK
- CLI 保留为备用入口，便于在没有 GUI 依赖时验证逻辑

## 目录结构

```text
02_memory_management
├── CMakeLists.txt
├── Makefile
├── README.md
└── src
    ├── cli_main.cpp
    ├── main.cpp
    ├── gui
    │   ├── BlocksTable.cpp
    │   ├── BlocksTable.h
    │   ├── MainWindow.cpp
    │   ├── MainWindow.h
    │   ├── MemoryLayoutWidget.cpp
    │   ├── MemoryLayoutWidget.h
    │   ├── PageStepsTable.cpp
    │   └── PageStepsTable.h
    └── memory
        ├── MemoryFacade.cpp
        ├── MemoryFacade.h
        ├── MemoryTypes.h
        ├── PageReplacement.cpp
        ├── PageReplacement.h
        ├── PartitionManager.cpp
        └── PartitionManager.h
```

## 已实现内容

- 动态分区分配
  - `FF`：首次适应
  - `BF`：最佳适应
- 页面置换
  - `FIFO`
  - `LRU`
  - `LFU`
  - `CLOCK`
  - `RANDOM`

## GUI 功能

FLTK 图形界面支持：

- 在“动态分区分配”和“页面置换”两种模式间切换
- 在动态分区模式下初始化内存、申请内存、释放内存、载入示例数据
- 在动态分区模式下随机生成总内存和申请/释放序列，并按时间间隔自动回放
- 通过图形化内存布局展示分区分配和回收结果
- 使用表格展示已分配分区和空闲分区
- 在页面置换模式下输入页框数量、访问序列并选择算法
- 在页面置换模式下随机生成页框数量和访问序列
- 使用表格展示逐步置换过程
- 在结果摘要区显示统计结果和详细步骤

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

目录下已经提供了 [Makefile](02_memory_management/Makefile)，推荐直接使用：

```bash
cd 02_memory_management
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
cd 02_memory_management
cmake -S . -B build -DBUILD_MEMORY_MANAGEMENT_GUI=ON -DBUILD_MEMORY_MANAGEMENT_CLI=ON
cmake --build build --target memory_management_gui -j
./build/memory_management_gui
```

## 编译运行 CLI

```bash
cd 02_memory_management
cmake -S . -B build-cli -DBUILD_MEMORY_MANAGEMENT_GUI=OFF -DBUILD_MEMORY_MANAGEMENT_CLI=ON
cmake --build build-cli --target memory_management_cli -j
./build-cli/memory_management_cli
```

## GUI 使用说明

1. 启动 `memory_management_gui`
2. 选择“动态分区分配”或“页面置换”模式
3. 动态分区模式中可初始化内存、申请/释放进程内存并查看内存布局
4. 也可以输入“随机步数”和“间隔ms”，点击“生成序列”后再点击“开始回放”，按固定时间间隔观察分区变化
5. 页面置换模式中输入页框数和访问序列，选择 `FIFO`、`LRU`、`LFU`、`CLOCK` 或 `RANDOM` 后运行模拟
6. 也可以输入“随机长度”后点击“随机生成输入”，自动生成页框数量和带局部性特征的访问序列
7. 在界面中查看表格结果、图形布局和摘要统计

## CLI 页面置换示例输入

```text
2
3
12
1 2 3 4 1 2 5 1 2 3 4 5
1
```

含义：

- 先进入页面置换模块
- 页框数为 3
- 页面访问序列长度为 12
- 最后选择 `1` 表示运行 FIFO
