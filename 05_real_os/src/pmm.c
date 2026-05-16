#include "pmm.h"
#include <stdbool.h>

/* 这个符号是在 linker.ld 中定义的，代表内核二进制代码在内存中的结束位置 */
extern uint32_t _kernel_end;

/* 物理内存位图：每一位(bit)代表一个 4KB 的物理页是否被占用 */
static uint8_t* memory_bitmap;
static uint32_t total_pages;
static uint32_t free_pages;

/* 位图操作辅助函数 */
static inline void bitmap_set(uint32_t bit) {
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_unset(uint32_t bit) {
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool bitmap_test(uint32_t bit) {
    return memory_bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm_init(uint32_t mem_upper_kb) {
    /* 
     * GRUB 报告的 mem_upper 是 1MB 以上的连续内存，单位是 KB。
     * 所以总物理内存 = mem_upper * 1024 + 1MB
     */
    uint32_t total_memory = (mem_upper_kb * 1024) + 0x100000;
    
    /* 总页数 */
    total_pages = total_memory / PMM_PAGE_SIZE;
    free_pages = total_pages;

    /* 将位图放在内核映像结束的地方 */
    memory_bitmap = (uint8_t*)&_kernel_end;
    
    /* 计算位图本身需要占据多少个字节 */
    uint32_t bitmap_size_bytes = total_pages / 8;
    if (total_pages % 8 != 0) {
        bitmap_size_bytes++;
    }

    /* 初始化：将所有内存标记为空闲 (0) */
    for (uint32_t i = 0; i < bitmap_size_bytes; i++) {
        memory_bitmap[i] = 0;
    }

    /* 
     * 关键步骤：保留已经被内核和位图本身占用的内存页！
     * 内核起始于 1MB (0x100000)，我们需要计算出内核+位图一共占用了多少页
     */
    uint32_t kernel_and_bitmap_end = (uint32_t)&_kernel_end + bitmap_size_bytes;
    uint32_t reserved_pages = kernel_and_bitmap_end / PMM_PAGE_SIZE;
    if (kernel_and_bitmap_end % PMM_PAGE_SIZE != 0) {
        reserved_pages++;
    }

    /* 将这些保留页在位图中标记为已占用 (1) */
    for (uint32_t i = 0; i < reserved_pages; i++) {
        bitmap_set(i);
        free_pages--;
    }
}

void* pmm_alloc_page(void) {
    if (free_pages == 0) {
        return NULL; /* 内存耗尽 (Out of Memory) */
    }

    /* 暴力遍历位图寻找第一个为 0 的 bit (首次适应算法) */
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_pages--;
            return (void*)(i * PMM_PAGE_SIZE); /* 返回页面的物理基地址 */
        }
    }
    
    return NULL;
}

void pmm_free_page(void* ptr) {
    uint32_t page_index = (uint32_t)ptr / PMM_PAGE_SIZE;
    if (bitmap_test(page_index)) {
        bitmap_unset(page_index);
        free_pages++;
    }
}

uint32_t pmm_get_free_pages(void) {
    return free_pages;
}
