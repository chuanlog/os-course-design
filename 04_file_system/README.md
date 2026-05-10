# 文件系统模拟

本模块已经改造成 `多文件分层 + FLTK 图形界面 + CLI 备用入口` 的结构：

- 文件系统核心逻辑位于 `src/filesystem/`
- GUI 使用 `FLTK` 实现，界面风格参考 Windows 文件管理器：左侧目录树、右侧文件列表、下方内容编辑区
- 所有持久化数据只读写 **一个二进制镜像文件**，默认文件名为 `virtual_disk.bin`
- GUI 通过 `CMake FetchContent` 自动拉取 FLTK

## 目录结构

```text
04_file_system
├── CMakeLists.txt
├── Makefile
├── README.md
└── src
    ├── cli_main.cpp
    ├── main.cpp
    ├── gui
    │   ├── MainWindow.cpp
    │   └── MainWindow.h
    └── filesystem
        ├── FileSystemFacade.cpp
        ├── FileSystemFacade.h
        ├── FsTypes.cpp
        ├── FsTypes.h
        ├── VirtualFileSystem.cpp
        └── VirtualFileSystem.h
```

## 核心设计

本文件系统采用单镜像文件设计，镜像内部包含：

- 镜像头
- 位示图
- 固定数量的目录项表
- 数据块区

目录项支持层次结构：

- 文件夹
- 文件
- 父子关系
- 创建/更新时间
- 文件大小
- 文件占用块列表

## 已实现功能

- 格式化磁盘镜像
- 加载单个二进制镜像文件
- 创建文件
- 创建文件夹
- 重命名文件或文件夹
- 删除文件或文件夹（文件夹递归删除）
- 进入目录 / 返回上一级
- 编辑并保存文件内容
- 读取文件内容
- 查看空间统计
- 查看位示图

## GUI 功能

FLTK 图形界面支持：

- 左侧目录树展示层级目录
- 右侧列表展示当前目录中的文件和文件夹
- 下方文本编辑区编辑选中文件内容
- 顶部输入镜像路径、总块数和块大小
- 点击“格式化磁盘”后初始化单个镜像文件
- 所有文件系统修改操作都会直接写回同一个二进制镜像文件

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

目录下已经提供了 [Makefile](file:///Users/bytedance/GitRepos/os-course-design/04_file_system/Makefile)，推荐直接使用：

```bash
cd /Users/bytedance/GitRepos/os-course-design/04_file_system
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
cd /Users/bytedance/GitRepos/os-course-design/04_file_system
cmake -S . -B build -DBUILD_FILE_SYSTEM_GUI=ON -DBUILD_FILE_SYSTEM_CLI=ON
cmake --build build --target file_system_gui -j
./build/file_system_gui
```

## 编译运行 CLI

```bash
cd /Users/bytedance/GitRepos/os-course-design/04_file_system
cmake -S . -B build-cli -DBUILD_FILE_SYSTEM_GUI=OFF -DBUILD_FILE_SYSTEM_CLI=ON
cmake --build build-cli --target file_system_cli -j
./build-cli/file_system_cli
```

## GUI 使用说明

1. 启动 `file_system_gui`
2. 在顶部输入镜像文件名、总块数和块大小
3. 点击“格式化磁盘”创建单个镜像文件，或点击“加载镜像”读取已有镜像
4. 在左侧目录树中选择目录，在右侧列表中查看当前目录对象
5. 使用“新建文件”“新建文件夹”“重命名”“删除”等按钮管理对象
6. 选中文件后可在下方编辑区修改内容，再点击“保存内容”写回镜像
7. 点击“位图”可查看当前空闲空间分布

## CLI 示例

```text
format 128 256
mkdir /docs
create /docs/readme.txt
write /docs/readme.txt hello_os_course
ls /docs
read /docs/readme.txt
stat
bitmap
exit
```

## 说明

- 当前模拟文件系统只读写一个镜像文件，不会把每个虚拟文件单独映射成宿主机上的真实文件。
- 这更适合课程设计中演示“目录项管理 + 位示图 + 数据块分配”的完整机制。
