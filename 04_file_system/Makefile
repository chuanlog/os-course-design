SHELL := /bin/bash

BUILD_DIR ?= build
CLI_BUILD_DIR ?= build-cli
CMAKE ?= cmake

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
JOBS ?= $(shell sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
JOBS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
endif

.DEFAULT_GOAL := help

.PHONY: help check-tools gui cli run run-cli clean distclean reconfigure reconfigure-cli

help:
	@echo "可用目标:"
	@echo "  make gui             编译 FLTK 图形界面"
	@echo "  make run             编译并运行 FLTK 图形界面"
	@echo "  make cli             编译命令行版本"
	@echo "  make run-cli         编译并运行命令行版本"
	@echo "  make clean           删除 GUI 构建目录"
	@echo "  make distclean       删除所有构建目录"
	@echo "  make reconfigure     重新配置并编译 GUI"
	@echo "  make reconfigure-cli 重新配置并编译 CLI"

check-tools:
	@command -v $(CMAKE) >/dev/null 2>&1 || { echo "错误: 未找到 cmake，请先安装 CMake 3.20+。"; exit 1; }
	@command -v git >/dev/null 2>&1 || { echo "错误: 未找到 git，请先安装 git。"; exit 1; }

$(BUILD_DIR)/CMakeCache.txt: | check-tools
	$(CMAKE) -S . -B $(BUILD_DIR) \
		-DBUILD_FILE_SYSTEM_GUI=ON \
		-DBUILD_FILE_SYSTEM_CLI=ON

$(CLI_BUILD_DIR)/CMakeCache.txt: | check-tools
	$(CMAKE) -S . -B $(CLI_BUILD_DIR) \
		-DBUILD_FILE_SYSTEM_GUI=OFF \
		-DBUILD_FILE_SYSTEM_CLI=ON

gui: $(BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(BUILD_DIR) --target file_system_gui -j $(JOBS)

cli: $(CLI_BUILD_DIR)/CMakeCache.txt
	$(CMAKE) --build $(CLI_BUILD_DIR) --target file_system_cli -j $(JOBS)

run: gui
	./$(BUILD_DIR)/file_system_gui

run-cli: cli
	./$(CLI_BUILD_DIR)/file_system_cli

clean:
	rm -rf $(BUILD_DIR)

distclean:
	rm -rf $(BUILD_DIR) $(CLI_BUILD_DIR)

reconfigure: clean gui

reconfigure-cli:
	rm -rf $(CLI_BUILD_DIR)
	$(MAKE) cli
