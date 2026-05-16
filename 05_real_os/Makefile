# 编译器设置：由于我们要编译纯净的无操作系统的 x86 目标文件，必须使用交叉编译器
CC = i686-elf-gcc
AS = i686-elf-as

# CFLAGS:
# -std=gnu99: 使用 GNU C99 标准
# -ffreestanding: 告诉编译器没有标准库，入口不是 main，我们需要自己提供一切
# -O2: 优化代码
# -Wall -Wextra: 开启所有警告
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# LDFLAGS:
# -nostdlib: 不链接标准库
# -lgcc: 链接底层的 GCC 支持库
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

# 目录
SRC_DIR = src
BUILD_DIR = build
ISODIR = isodir

# 目标文件
OBJS = $(BUILD_DIR)/boot.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/pmm.o $(BUILD_DIR)/ata.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/kernel.o

# 默认目标：编译出 ISO 镜像
all: minios.iso disk.img

# 编译 C 文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# 编译汇编文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

# 链接生成内核可执行文件 (ELF格式)
minios.bin: $(OBJS) linker.ld
	$(CC) -T linker.ld -o $@ $(LDFLAGS) $(OBJS)

# 将内核打包成可启动的 ISO 镜像
minios.iso: minios.bin
	@mkdir -p $(ISODIR)/boot/grub
	cp minios.bin $(ISODIR)/boot/minios.bin
	# i686-elf-grub-mkrescue 会根据 isodir 目录的结构和 grub.cfg 自动生成支持光盘引导的 ISO 文件
	i686-elf-grub-mkrescue -o minios.iso $(ISODIR)

# 创建一个 2MB 的空白虚拟硬盘文件，用于文件系统测试
disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=2

# 清理编译产物
clean:
	rm -rf $(BUILD_DIR) minios.bin minios.iso disk.img $(ISODIR)/boot/minios.bin

# 在 QEMU 虚拟机中运行 ISO 进行测试 (缩放显示)，并挂载硬盘
run: minios.iso disk.img
	qemu-system-i386 -cdrom minios.iso -drive file=disk.img,format=raw,index=0,media=disk -display cocoa,zoom-to-fit=on
