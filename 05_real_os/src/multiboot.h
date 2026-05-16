#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include <stdint.h>

/* GRUB 启动时传给内核的信息结构体 */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper; /* 我们主要需要这个：1MB 以上的物理内存大小（KB为单位） */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
};

#endif