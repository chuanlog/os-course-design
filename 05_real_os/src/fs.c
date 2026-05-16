#include "fs.h"
#include "ata.h"
#include <stdint.h>

extern void terminal_writestring(const char* data);
extern void terminal_putchar(char c);

/* 文件系统魔数，升级为 V2 以触发格式化 */
#define FS_MAGIC 0x4D696E32 /* "Min2" */

/* 最大条目数 (文件 + 目录) */
#define MAX_ENTRIES 30

/* 文件元数据表项 */
struct file_entry {
    char name[16];
    uint32_t start_lba;
    uint32_t size;
    uint8_t is_dir;       /* 1 为目录，0 为文件 */
    int32_t parent_idx;   /* 父目录的条目索引，根目录为 -1 */
};

/* 超级块 */
struct superblock {
    uint32_t magic;
    uint32_t entry_count;
    struct file_entry entries[MAX_ENTRIES];
};

static struct superblock sb;
static int32_t current_dir_idx = 0; /* 当前目录，默认 0 为根目录 "/" */

/* 内部工具函数 */
static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void strcpy(char* dest, const char* src) {
    while((*dest++ = *src++));
}

static int strlen(const char* s) {
    int len = 0;
    while(s[len]) len++;
    return len;
}

static void memset(void* dest, uint8_t val, uint32_t count) {
    uint8_t* d = (uint8_t*)dest;
    while (count--) {
        *d++ = val;
    }
}

/* 为了支持更多的文件目录（MAX_ENTRIES=30），超级块超过了 512 字节（2个扇区大小）。
 * 这里将其提升为 2048 字节 (4个扇区)。
 */
static void fs_sync_sb(void) {
    uint8_t sb_buffer[2048];
    memset(sb_buffer, 0, 2048);
    uint32_t copy_size = sizeof(struct superblock) > 2048 ? 2048 : sizeof(struct superblock);
    for(uint32_t i = 0; i < copy_size; i++) {
        sb_buffer[i] = ((uint8_t*)&sb)[i];
    }
    
    /* 循环写入 4 个扇区 (LBA 1, 2, 3, 4) */
    for (int i = 0; i < 4; i++) {
        ata_write_sector(1 + i, sb_buffer + (i * 512));
    }
}

void fs_init(void) {
    uint8_t buffer[2048];
    /* 循环读取 4 个扇区 (LBA 1, 2, 3, 4) */
    for (int i = 0; i < 4; i++) {
        ata_read_sector(1 + i, buffer + (i * 512));
    }
    
    struct superblock* disk_sb = (struct superblock*)buffer;
    
    if (disk_sb->magic == FS_MAGIC) {
        uint32_t copy_size = sizeof(struct superblock) > 2048 ? 2048 : sizeof(struct superblock);
        for(uint32_t i = 0; i < copy_size; i++) {
            ((uint8_t*)&sb)[i] = buffer[i];
        }
        current_dir_idx = 0;
        terminal_writestring("       [SUCCESS] MiniFS V2 (Hierarchical) loaded.\n");
    } else {
        terminal_writestring("       [INFO] Old or Unformatted disk. Formatting to V2...\n");
        fs_format();
    }
}

void fs_format(void) {
    sb.magic = FS_MAGIC;
    sb.entry_count = 1;
    memset(sb.entries, 0, sizeof(sb.entries));
    
    /* 创建根目录 */
    strcpy(sb.entries[0].name, "/");
    sb.entries[0].is_dir = 1;
    sb.entries[0].parent_idx = -1; /* 根目录没有父目录 */
    
    fs_sync_sb();
    current_dir_idx = 0;
    terminal_writestring("       [SUCCESS] Disk formatted with MiniFS V2.\n");
}

void fs_get_cwd(char* buf) {
    if (current_dir_idx == 0) {
        strcpy(buf, "/");
        return;
    }
    int32_t path_indices[20];
    int depth = 0;
    int32_t curr = current_dir_idx;
    while (curr > 0 && depth < 20) {
        path_indices[depth++] = curr;
        curr = sb.entries[curr].parent_idx;
    }
    
    int pos = 0;
    for (int i = depth - 1; i >= 0; i--) {
        buf[pos++] = '/';
        char* n = sb.entries[path_indices[i]].name;
        while (*n) {
            buf[pos++] = *n++;
        }
    }
    buf[pos] = '\0';
}

void fs_pwd(void) {
    char path[256];
    fs_get_cwd(path);
    terminal_writestring(path);
    terminal_writestring("\n");
}

void fs_mkdir(const char* name) {
    if (sb.entry_count >= MAX_ENTRIES) {
        terminal_writestring("Error: Disk directory is full.\n");
        return;
    }
    for (uint32_t i = 0; i < sb.entry_count; i++) {
        if (sb.entries[i].parent_idx == current_dir_idx && strcmp(sb.entries[i].name, name) == 0) {
            terminal_writestring("Error: Directory or file already exists.\n");
            return;
        }
    }
    
    struct file_entry* fe = &sb.entries[sb.entry_count];
    strcpy(fe->name, name);
    fe->is_dir = 1;
    fe->parent_idx = current_dir_idx;
    fe->size = 0;
    fe->start_lba = 0;
    
    sb.entry_count++;
    fs_sync_sb();
}

void fs_cd(const char* name) {
    if (strcmp(name, "/") == 0) {
        current_dir_idx = 0;
        return;
    }
    if (strcmp(name, "..") == 0) {
        if (current_dir_idx != 0) {
            current_dir_idx = sb.entries[current_dir_idx].parent_idx;
        }
        return;
    }
    for (uint32_t i = 0; i < sb.entry_count; i++) {
        if (sb.entries[i].parent_idx == current_dir_idx && 
            sb.entries[i].is_dir && 
            strcmp(sb.entries[i].name, name) == 0) {
            current_dir_idx = i;
            return;
        }
    }
    terminal_writestring("cd: no such directory\n");
}

void fs_list(void) {
    int count = 0;
    for (uint32_t i = 0; i < sb.entry_count; i++) {
        if (i == 0) continue; // Skip root directory entry itself
        if (sb.entries[i].parent_idx == current_dir_idx) {
            terminal_writestring(sb.entries[i].name);
            if (sb.entries[i].is_dir) {
                terminal_writestring("/");
            }
            terminal_writestring("  ");
            count++;
        }
    }
    if (count == 0) {
        terminal_writestring("(empty)");
    }
    terminal_writestring("\n");
}

void fs_write_file(const char* name, const char* content) {
    if (sb.entry_count >= MAX_ENTRIES) {
        terminal_writestring("Error: Disk directory is full.\n");
        return;
    }
    
    for (uint32_t i = 0; i < sb.entry_count; i++) {
        if (sb.entries[i].parent_idx == current_dir_idx && strcmp(sb.entries[i].name, name) == 0) {
            terminal_writestring("Error: File already exists.\n");
            return;
        }
    }
    
    /* 数据存放在 LBA 5, 6, 7... 以此类推，避开超级块占用的前 4 个扇区 */
    uint32_t lba = 5 + sb.entry_count;
    struct file_entry* fe = &sb.entries[sb.entry_count];
    strcpy(fe->name, name);
    fe->is_dir = 0;
    fe->parent_idx = current_dir_idx;
    fe->start_lba = lba;
    fe->size = strlen(content);
    
    uint8_t buffer[512];
    memset(buffer, 0, 512);
    strcpy((char*)buffer, content);
    ata_write_sector(lba, buffer);
    
    sb.entry_count++;
    fs_sync_sb();
    
    terminal_writestring("File successfully written to disk.\n");
}

void fs_read_file(const char* name) {
    for (uint32_t i = 0; i < sb.entry_count; i++) {
        if (sb.entries[i].parent_idx == current_dir_idx && 
            !sb.entries[i].is_dir && 
            strcmp(sb.entries[i].name, name) == 0) {
            
            uint8_t buffer[512];
            ata_read_sector(sb.entries[i].start_lba, buffer);
            
            for(uint32_t j = 0; j < sb.entries[i].size; j++) {
                terminal_putchar(buffer[j]);
            }
            terminal_writestring("\n");
            return;
        }
    }
    terminal_writestring("Error: File not found.\n");
}