#include "loader.h"
#include "fs.h"
#include "ata.h"
#include "pmm.h"

extern void terminal_writestring(const char* data);

/* 内部字符串工具 */
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* 这部分我们需要借用 fs.c 里的数据结构来解析文件大小和位置 */
#define MAX_ENTRIES 30
struct file_entry {
    char name[16];
    uint32_t start_lba;
    uint32_t size;
    uint8_t is_dir;
    int32_t parent_idx;
};

struct superblock {
    uint32_t magic;
    uint32_t entry_count;
    struct file_entry entries[MAX_ENTRIES];
};

void loader_exec(const char* filename) {
    /* 1. 读取超级块，寻找文件 */
    uint8_t sb_buffer[2048];
    for (int i = 0; i < 4; i++) {
        ata_read_sector(1 + i, sb_buffer + (i * 512));
    }
    struct superblock* sb = (struct superblock*)sb_buffer;
    
    struct file_entry* target = 0;
    for (uint32_t i = 0; i < sb->entry_count; i++) {
        if (!sb->entries[i].is_dir && strcmp(sb->entries[i].name, filename) == 0) {
            target = &sb->entries[i];
            break;
        }
    }
    
    if (!target) {
        terminal_writestring("exec: file not found or is a directory.\n");
        return;
    }
    
    /* 2. 我们使用固定的地址来加载应用程序，以保证 Flat Binary 的绝对地址寻址正常工作
     * 假设我们固定将应用加载到 0x1000000 (16MB) 处。这需要确保该地址对应的物理页可用。
     * 为了兼容 PMM，我们可以临时将其标记为已使用，或者因为我们单任务直接覆盖使用。
     * 这里为了安全，我们用 pmm_alloc_page() 分配。
     * 但因为之前 flat binary 用了绝对地址，所以它必须和编译时指定的 -Ttext 对应！
     * 在新的 C 语言架构中，我们将采用位置无关或固定地址，这里固定用 0x1000000。
     */
    uint8_t* load_addr = (uint8_t*)0x1000000;
    
    /* 3. 将程序从硬盘读入物理内存 */
    /* 因为我们的文件系统比较简陋，文件数据存储在连续的 LBA 中 */
    uint32_t sectors_to_read = target->size / 512;
    if (target->size % 512 != 0) sectors_to_read++;
    
    for (uint32_t i = 0; i < sectors_to_read; i++) {
        ata_read_sector(target->start_lba + i, load_addr + (i * 512));
    }
    
    terminal_writestring("[INFO] Jumping to application code...\n");
    
    /* 4. 定义内核 API 结构体，供应用程序调用 */
    extern char kbd_getchar(void);
    /* [修复点] 我们不能强转为固定两个参数的函数指针，而是直接使用汇编级别的跳转！
     * 在 GCC 中，如果用 C 语言强转函数指针调用，GCC 可能会自作主张地加入栈帧保护，
     * 导致裸机环境下栈错乱卡死。
     */
    extern volatile int app_running;
    app_running = 1;
    
    /* 将内核 API 的地址通过寄存器传给应用程序，避免 C 语言调用约定的栈破坏 */
    __asm__ volatile (
        "pushl %0 \n"  /* 压入第二个参数：getchar */
        "pushl %1 \n"  /* 压入第一个参数：print */
        "call *%2 \n"  /* 调用应用程序入口 */
        "addl $8, %%esp \n" /* 清理栈 */
        : 
        : "r"(kbd_getchar), "r"(terminal_writestring), "r"(load_addr)
        : "memory"
    );
    
    app_running = 0;
    
    /* 5. 程序执行完毕返回，清理 */
    terminal_writestring("\n[INFO] Application exited.\n");
}