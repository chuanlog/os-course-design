# 编译器设置：由于我们要编译纯净的无操作系统的 x86 目标文件，必须使用交叉编译器。
# GNU Make 内置默认 CC=cc / LD=ld；若未显式覆盖，则替换为 i686 交叉工具链。
ifeq ($(origin CC), default)
CC := i686-elf-gcc
endif

ifeq ($(origin LD), default)
LD := i686-elf-ld
endif

GRUB_MKRESCUE ?= i686-elf-grub-mkrescue
QEMU ?= qemu-system-i386

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
GRUB_CFG = grub.cfg

# 目标文件
OBJS = $(BUILD_DIR)/boot.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/pmm.o $(BUILD_DIR)/ata.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/shell.o $(BUILD_DIR)/loader.o $(BUILD_DIR)/kernel.o

# 自动扫描 apps 目录下的所有 .S 和 .c 文件
# app_crt0.S 是 C 应用运行时入口，不作为独立应用打包。
APP_ASM_SRCS := $(filter-out apps/app_crt0.S,$(wildcard apps/*.S))
APP_C_SRCS := $(wildcard apps/*.c)
APP_BINS := $(patsubst apps/%.S, %.bin, $(APP_ASM_SRCS)) $(patsubst apps/%.c, %.bin, $(APP_C_SRCS))

.PHONY: all check-tools clean run

# 默认目标：编译出 ISO 镜像
all: check-tools minios.iso disk.img

check-tools:
	@command -v $(CC) >/dev/null 2>&1 || { echo "错误: 未找到 $(CC)，请先安装 i686-elf-gcc 交叉编译工具链。"; exit 1; }
	@command -v $(LD) >/dev/null 2>&1 || { echo "错误: 未找到 $(LD)，请先安装 i686-elf-binutils。"; exit 1; }
	@command -v $(GRUB_MKRESCUE) >/dev/null 2>&1 || { echo "错误: 未找到 $(GRUB_MKRESCUE)，请先安装 i686-elf-grub。"; exit 1; }

# 编译前自动生成应用元数据头文件
$(BUILD_DIR)/fs.o: $(SRC_DIR)/fs.c $(SRC_DIR)/apps_meta.h

$(SRC_DIR)/apps_meta.h: $(APP_BINS)
	@echo "Generating apps_meta.h..."
	@echo "#ifndef APPS_META_H" > $@
	@echo "#define APPS_META_H" >> $@
	@echo "#include <stdint.h>" >> $@
	@echo "struct prebuilt_app { char name[16]; uint32_t start_lba; uint32_t size; };" >> $@
	@echo "static struct prebuilt_app prebuilt_apps[] = {" >> $@
	@LBA=5; \
	for bin in $(APP_BINS); do \
		NAME=$$(basename $$bin); \
		SIZE=$$(wc -c < $$bin | tr -d ' '); \
		echo "    {\"$$NAME\", $$LBA, $$SIZE}," >> $@; \
		LBA=$$((LBA + (SIZE + 511) / 512)); \
	done
	@echo "};" >> $@
	@echo "#define PREBUILT_APP_COUNT (sizeof(prebuilt_apps)/sizeof(prebuilt_apps[0]))" >> $@
	@echo "#endif" >> $@

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
minios.iso: minios.bin $(GRUB_CFG)
	@mkdir -p $(ISODIR)/boot/grub
	cp minios.bin $(ISODIR)/boot/minios.bin
	cp $(GRUB_CFG) $(ISODIR)/boot/grub/grub.cfg
	# i686-elf-grub-mkrescue 会根据 isodir 目录的结构和 grub.cfg 自动生成支持光盘引导的 ISO 文件
	$(GRUB_MKRESCUE) -o minios.iso $(ISODIR)

# 创建虚拟硬盘并动态注入所有编译好的 APP
disk.img: $(APP_BINS)
	@echo "Building disk.img with apps..."
	@dd if=/dev/zero of=disk.img bs=1M count=2 2>/dev/null
	@LBA=5; \
	for bin in $(APP_BINS); do \
		SIZE=$$(wc -c < $$bin | tr -d ' '); \
		dd if=$$bin of=disk.img bs=512 seek=$$LBA conv=notrunc 2>/dev/null; \
		LBA=$$((LBA + (SIZE + 511) / 512)); \
	done

# 自动编译外部测试程序 (纯二进制，针对汇编)
%.bin: apps/%.S
	@mkdir -p $(BUILD_DIR)
	$(CC) -m32 -c $< -o $(BUILD_DIR)/$*.o
	$(LD) -m elf_i386 -Ttext 0x1000000 --oformat binary -o $@ $(BUILD_DIR)/$*.o

# 自动编译外部测试程序 (纯二进制，针对 C 语言)
%.bin: apps/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -m32 -c apps/app_crt0.S -o $(BUILD_DIR)/app_crt0.o
	$(CC) -m32 -c $< -o $(BUILD_DIR)/$*.o $(CFLAGS) -fno-pie -fno-pic -fno-stack-protector -fno-builtin -mno-sse -mno-mmx -mno-sse2
	$(LD) -m elf_i386 -Ttext 0x1000000 --oformat binary -e _start -o $@ $(BUILD_DIR)/app_crt0.o $(BUILD_DIR)/$*.o

# 清理编译产物
clean:
	rm -rf $(BUILD_DIR) minios.bin minios.iso disk.img *.bin $(SRC_DIR)/apps_meta.h $(ISODIR)/boot/minios.bin

# 在 QEMU 虚拟机中运行 ISO 进行测试 (缩放显示)，并挂载硬盘
run: check-tools minios.iso disk.img
	@command -v $(QEMU) >/dev/null 2>&1 || { echo "错误: 未找到 $(QEMU)，请先安装 QEMU。"; exit 1; }
	$(QEMU) -cdrom minios.iso -drive file=disk.img,format=raw,index=0,media=disk -display cocoa,zoom-to-fit=on
