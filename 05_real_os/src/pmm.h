#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

/* 物理内存页大小：4KB */
#define PMM_PAGE_SIZE 4096

/* 初始化物理内存管理器 */
void pmm_init(uint32_t mem_upper_kb);

/* 分配一个 4KB 的物理页，返回其物理地址 */
void* pmm_alloc_page(void);

/* 释放一个 4KB 的物理页 */
void pmm_free_page(void* ptr);

/* 获取当前剩余的空闲页数 */
uint32_t pmm_get_free_pages(void);

#endif