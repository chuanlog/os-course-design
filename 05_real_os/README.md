# MiniOS 真实裸机操作系统

本目录实现了课程设计拓展部分的真实 x86 裸机操作系统 `MiniOS`。它不是运行在宿主机上的模拟器，而是可以编译为可启动 `ISO` 镜像，并通过 `GRUB Multiboot` 引导进入 32 位保护模式的极简操作系统。

系统已经实现：

- 可启动 `minios.iso` 镜像
- `QEMU` 虚拟机一键运行
- 可挂载虚拟硬盘 `disk.img`
- VGA 文本模式终端输出
- GDT / IDT / PIC / 键盘中断
- 物理内存页分配器 `PMM`
- ATA PIO 硬盘扇区读写
- 支持多级目录的 `MiniFS`
- 类 Linux Shell 命令
- 外部应用程序自动编译、自动注入磁盘、自动注册到文件系统
- 可运行外部汇编程序和 C 语言程序
- 交互式计算器 `calc.bin`

## 目录结构

```text
05_real_os
├── Makefile                 # 构建、打包 ISO、生成磁盘镜像、运行 QEMU
├── linker.ld                # 内核链接脚本，控制内核加载地址和段布局
├── minios.iso               # 构建生成的可启动 ISO 镜像
├── disk.img                 # 构建生成的虚拟硬盘镜像
├── minios.bin               # 构建生成的内核 ELF 文件
├── hello.bin                # 构建生成的外部汇编应用
├── calc.bin                 # 构建生成的外部 C 语言计算器应用
├── apps                     # 外部应用源码目录
│   ├── app_crt0.S           # C 应用运行时入口，保证 flat binary 从正确入口执行
│   ├── calc.c               # 交互式四则运算计算器
│   └── hello.S              # 外部汇编演示程序
├── isodir
│   └── boot
│       ├── grub
│       │   └── grub.cfg     # GRUB 启动配置
│       └── minios.bin       # 被打包进 ISO 的内核文件
└── src                      # 内核源码
    ├── apps_meta.h          # 构建时自动生成的应用元数据，不建议手写修改
    ├── ata.c / ata.h        # ATA PIO 硬盘驱动
    ├── boot.S               # Multiboot 入口、内核栈设置、跳转 C 内核
    ├── fs.c / fs.h          # MiniFS V2 文件系统
    ├── gdt.c / gdt.h        # GDT 全局描述符表
    ├── idt.c / idt.h        # IDT 中断描述符表和 PIC 重映射
    ├── interrupt.S          # GDT/IDT 汇编辅助函数、键盘 ISR
    ├── io.h                 # inb/outb 端口 I/O 封装
    ├── kernel.c             # 内核主入口、VGA 终端、系统初始化
    ├── keyboard.c           # PS/2 键盘驱动、Shift 支持、应用输入 API
    ├── loader.c / loader.h  # 外部程序加载器
    ├── multiboot.h          # Multiboot 信息结构
    ├── pmm.c / pmm.h        # 物理内存页分配器
    └── shell.c / shell.h    # 交互式 Shell
```

说明：

- `build/`、`*.bin`、`minios.iso`、`disk.img`、`src/apps_meta.h` 都是构建产物。
- 新增外部应用时，优先修改 `apps/` 目录，不需要手动修改 `fs.c` 注册应用。
- `src/apps_meta.h` 由 `Makefile` 自动生成，用于告诉内核每个应用在磁盘上的文件名、起始 LBA 和大小。

## 编译前准备

本项目面向 `i386` 32 位裸机环境，不能直接使用 macOS 自带的 `clang` 编译，需要交叉编译工具链。

推荐环境：

- `macOS`
- `Homebrew`
- `i686-elf-gcc`
- `i686-elf-binutils`
- `i686-elf-grub`
- `xorriso`
- `mtools`
- `qemu`

安装示例：

```bash
brew install i686-elf-binutils i686-elf-gcc i686-elf-grub
brew install xorriso mtools qemu
```

如果使用 VMware 运行，建议仍然先用 `QEMU` 完成开发验证，因为当前 `Makefile` 已经自动挂载了 `disk.img`，可以完整测试文件系统和外部应用加载。

## 一键编译运行

进入本目录：

```bash
cd /Users/bytedance/GitRepos/os-course-design/05_real_os
```

编译内核、打包 ISO、生成虚拟硬盘：

```bash
make clean
make
```

一键启动 QEMU：

```bash
make run
```

`make run` 实际会执行：

```bash
qemu-system-i386 \
  -cdrom minios.iso \
  -drive file=disk.img,format=raw,index=0,media=disk \
  -display cocoa,zoom-to-fit=on
```

启动后会进入 `MiniOS` Shell，提示符类似：

```text
root@minios:/#
```

## VMware 运行方式

`minios.iso` 是标准可启动光盘镜像，可以挂载到 VMware 的虚拟光驱中启动。

如果只验证系统能启动：

1. 新建一个 32 位 x86 虚拟机。
2. 将 `minios.iso` 挂载为 CD/DVD。
3. 设置从光驱启动。
4. 启动后即可看到 `MiniOS` 欢迎界面和 Shell。

如果要验证文件系统和外部应用，需要同时挂载虚拟硬盘。当前默认硬盘镜像是 raw 格式的 `disk.img`，可使用 `qemu-img` 转换为 VMDK 后再挂载：

```bash
cd /Users/bytedance/GitRepos/os-course-design/05_real_os
qemu-img convert -f raw -O vmdk disk.img disk.vmdk
```

然后在 VMware 中添加已有虚拟磁盘 `disk.vmdk`。推荐将其挂载为 IDE 磁盘，以匹配当前 ATA PIO 驱动使用的主 IDE 端口。

## 系统启动流程

整体启动链路如下：

```text
BIOS / 虚拟机固件
    ↓
GRUB Multiboot
    ↓
src/boot.S
    ↓
kernel_main()
    ↓
初始化 GDT / IDT / PIC / 键盘 / PMM / ATA / MiniFS
    ↓
进入 Shell
```

### 1. GRUB 引导

`isodir/boot/grub/grub.cfg` 指定启动项：

```text
multiboot /boot/minios.bin
boot
```

`grub-mkrescue` 会将 `isodir/` 打包成可启动 `ISO`。GRUB 负责从 16 位实模式切换到 32 位保护模式，并按 Multiboot 规范把内核加载到内存。

### 2. 汇编入口 `boot.S`

`src/boot.S` 提供 Multiboot 头、设置内核栈，并把 GRUB 传入的 `magic` 和 `multiboot_info` 参数转交给 C 语言入口 `kernel_main()`。

### 3. C 内核入口 `kernel.c`

`kernel_main()` 完成系统初始化：

- 初始化 VGA 文本终端
- 初始化 GDT
- 初始化 IDT 和 PIC
- 注册键盘中断 `IRQ1 -> INT 33`
- 开启 CPU 中断
- 初始化物理内存管理器
- 初始化硬盘和文件系统
- 进入 Shell 主循环

## 已实现功能

### 1. VGA 文本终端

文件：[kernel.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/kernel.c)

终端直接写入 VGA 文本显存 `0xB8000`，每个字符占 2 字节：

- 低 8 位：ASCII 字符
- 高 8 位：前景色和背景色

已支持：

- 字符输出
- 字符串输出
- 十进制和十六进制数字输出
- 回车换行
- 退格删除
- 到达底部后向上滚动
- VGA 硬件光标跟随当前位置
- QEMU 窗口缩放显示

注意：当前终端未实现 `\t` 制表符语义，排版应使用空格。

### 2. GDT / IDT / PIC / 中断

相关文件：

- [gdt.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/gdt.c)
- [idt.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/idt.c)
- [interrupt.S](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/interrupt.S)
- [keyboard.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/keyboard.c)

实现内容：

- GDT 中定义内核代码段和数据段
- 使用 `lgdt` 加载 GDT
- 使用远跳转刷新 `CS`
- IDT 中注册 256 个中断门
- PIC 重映射到 `0x20~0x2F`
- 键盘 IRQ1 映射为 `INT 33`
- 默认中断处理器防止未知中断造成 Triple Fault

键盘驱动支持：

- 普通字符输入
- 回车、退格、空格
- 左右 Shift 状态
- `Shift+=` 输入 `+`
- `Shift+8` 输入 `*`
- 应用运行期间将输入送入 `kbd_getchar()`
- Shell 运行期间将输入送入 `shell_input()`

### 3. 物理内存管理器 PMM

文件：`pmm.c`

PMM 将物理内存按 `4KB` 页管理，使用位图记录每一页是否空闲。

核心接口：

- `pmm_init(mem_upper_kb)`
- `pmm_alloc_page()`
- `pmm_free_page(ptr)`
- `pmm_get_free_pages()`

关键机制：

- 从 GRUB 的 Multiboot 信息中读取可用内存大小
- 计算物理页数量
- 位图紧跟内核镜像后方保存
- 初始化时保留 `0 ~ 内核结束 + 位图大小` 的内存区域
- 避免应用加载覆盖内核或 PMM 位图

### 4. ATA PIO 硬盘驱动

文件：[ata.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/ata.c)

当前使用主 IDE 通道的 ATA PIO 模式，通过端口 I/O 读写硬盘扇区。

常用端口：

- `0x1F0`：数据端口
- `0x1F2`：扇区数量
- `0x1F3~0x1F5`：LBA 地址
- `0x1F6`：驱动器和 LBA 高位
- `0x1F7`：命令/状态端口

已实现：

- `ata_read_sector(lba, buffer)`
- `ata_write_sector(lba, buffer)`

每次读写以 `512` 字节扇区为单位。

### 5. MiniFS V2 文件系统

文件：[fs.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/fs.c)

`MiniFS V2` 是一个简化的层次文件系统，元数据保存在虚拟硬盘的前几个扇区。

磁盘布局：

```text
LBA 0       保留
LBA 1~4     超级块 Superblock 和目录项表
LBA 5+      文件数据区 / 外部应用程序数据区
```

核心结构：

- `superblock`
  - 文件系统魔数
  - 目录项数量
  - 固定大小目录项数组
- `file_entry`
  - 文件名
  - 起始 LBA
  - 文件大小
  - 是否目录
  - 父目录索引

已支持：

- 自动格式化旧磁盘或空磁盘
- 根目录 `/`
- 多级目录
- 当前工作目录
- 文件创建与写入
- 文件读取
- 目录切换
- 外部应用文件自动注册

### 6. Shell 命令

文件：[shell.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/src/shell.c)

支持命令：

```text
help                 查看命令列表
clear                清屏
ls                   列出当前目录文件和目录
mkdir <dir>          创建目录
cd <dir>             切换目录
cd ..                返回上一级目录
cd /                 返回根目录
pwd                  输出当前路径
format               格式化 MiniFS
cat <file>           读取文件
write <file> <text>  写入文件
exec <file>          加载并执行外部程序
```

示例：

```text
mkdir home
cd home
write hello.txt hello_minios
ls
cat hello.txt
pwd
cd /
```

### 7. 外部程序加载器

文件：`load.c`

外部程序加载流程：

```text
Shell 输入 exec calc.bin
    ↓
loader_exec("calc.bin")
    ↓
读取 MiniFS 超级块，查找 calc.bin 元数据
    ↓
根据 start_lba 和 size 从 disk.img 读取程序内容
    ↓
加载到固定物理地址 0x1000000
    ↓
向应用传入内核 API：print / getchar
    ↓
call 0x1000000
    ↓
应用 ret 返回内核
```

当前采用固定加载地址 `0x1000000`，这样外部 C 应用中的绝对地址和链接地址一致，避免 flat binary 指针错位。

内核向应用传入两个函数指针：

- `terminal_writestring`：输出英文提示、结果等字符串
- `kbd_getchar`：阻塞读取一个键盘字符

这相当于一个非常早期的“系统调用雏形”。应用并不直接依赖内核内部变量，而是通过内核提供的 API 完成输入输出。

## 外部应用机制

外部应用源码位于 `apps/` 目录。

当前支持两类应用：

- 汇编应用：`apps/*.S`
- C 语言应用：`apps/*.c`

构建时 `Makefile` 会自动：

1. 扫描 `apps/*.S` 和 `apps/*.c`
2. 编译为 `*.o`
3. 链接为去掉 ELF 头的 flat binary：`*.bin`
4. 从 LBA 5 开始依次写入 `disk.img`
5. 生成 `src/apps_meta.h`
6. 内核格式化文件系统时自动注册这些应用

因此新增应用不需要手动修改 `fs.c`。

### 汇编应用规则

示例：`hello.S`
汇编应用需要：

- 提供 `_start` 符号
- 执行完毕后使用 `ret` 返回内核
- 如果需要显示，可以直接操作 VGA 显存或调用未来扩展的内核 API

最小示例：

```asm
.section .text
.global _start

_start:
    /* do something */
    ret
```

### C 语言应用规则

示例：[calc.c](file:///Users/bytedance/GitRepos/os-course-design/05_real_os/apps/calc.c)

C 应用不要自己写 `_start`，而是实现 `app_main`：

```c
typedef void (*print_t)(const char*);
typedef char (*getchar_t)(void);

void app_main(print_t print, getchar_t getchar) {
    print("Hello from C app!\n");
}
```

原因：

- C 编译器可能把辅助函数排在二进制开头
- flat binary 没有 ELF 头，也没有运行时加载器
- `apps/app_crt0.S` 会被强制链接到 C 应用最前面
- `app_crt0.S` 的 `_start` 位于文件偏移 `0`
- 它负责接收内核传入的 `print/getchar`，再调用 `app_main`

### 添加新应用 SOP

以新增 `demo.c` 为例：

1. 在 `apps/` 下新建文件：

```bash
cd /Users/bytedance/GitRepos/os-course-design/05_real_os
touch apps/demo.c
```

1. 编写应用：

```c
typedef void (*print_t)(const char*);
typedef char (*getchar_t)(void);

void app_main(print_t print, getchar_t getchar) {
    print("Demo app started.\n");
    print("Press any key to return: ");
    getchar();
    print("\nBye.\n");
}
```

1. 重新编译运行：

```bash
make clean
make run
```

1. 在 MiniOS 中执行：

```text
ls
exec demo.bin
```

## 交互式计算器

计算器是一个内置的系统应用，是这个操作系统上跑程序的一个例子
运行方式：

```text
exec calc.bin
```

功能：

- 英文提示界面
- 输入第一个操作数
- 选择运算符
- 输入第二个操作数
- 输出计算结果
- 支持循环计算
- 输入 `q` 退出

支持运算：

- `+` 加法
- `-` 减法
- `*` 乘法
- `/` 整数除法
- 除 0 错误提示

示例：

```text
Enter first number (or 'q' to quit): 12
Enter operation (+, -, *, /): +
Enter second number: 30
Result: 12 + 30 = 42
```

输入说明：

- 主键盘 `+`：使用 `Shift+=`
- 主键盘 `*`：使用 `Shift+8`
- 小键盘 `+`、`*` 也可用

## 构建系统过程

`Makefile` 主要目标：

```text
make          编译内核、应用、ISO 和磁盘镜像
make run      使用 QEMU 启动 MiniOS
make clean    清理构建产物
```

构建过程：

```text
src/*.c / src/*.S
    ↓
build/*.o
    ↓
linker.ld 链接
    ↓
minios.bin
    ↓
grub-mkrescue
    ↓
minios.iso

apps/*.S / apps/*.c
    ↓
*.bin flat binary
    ↓
dd 写入 disk.img 的连续 LBA
    ↓
生成 src/apps_meta.h
    ↓
MiniFS 自动注册应用文件
```
